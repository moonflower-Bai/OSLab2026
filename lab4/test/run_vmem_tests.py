import json
import math
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
from typing import NamedTuple, Optional


TEST = Path(__file__).resolve().parent
LAB = TEST.parent
PMM = LAB / "kern/mm/pmm.c"
HEADERS = [LAB / path for path in ("kern/mm", "kern/debug", "kern/driver", "kern/sync", "libs")]
C_FLAGS = ["-std=gnu11", "-O2", "-ffreestanding", "-fno-builtin", "-fno-stack-protector",
           "-ffunction-sections", "-fdata-sections"]
UNIT = "单元测试"
GUEST = "QEMU 集成测试"
MODEL = "模型仿真测试"
PASS = "通过"
FAIL = "错误"
SKIP = "未运行"
UNIT_CASES = (
    "first-map", "shared-alias", "same-page-permission", "replace-page", "remove-last-repeat",
    "remove-missing-path", "remove-invalid-leaf", "alloc-fail-first", "alloc-fail-second",
)
GUEST_CASES = ("replace-and-read", "protect-and-revoke")
SEEDS = ("0x13579bdf", "0x9e3779b9", "0x243f6a88", "0xdeadbeef")

PAGE_FENCE = re.compile(
    r'\b(?:asm|__asm__)\s+(?:volatile|__volatile__)\s*\(\s*'
    r'"sfence\.vma\s+%0(?:\s*,\s*(?:zero|x0))?"\s*:\s*:\s*'
    r'"r"\(([^;\n]+?)\)\s*(?::\s*"memory"\s*)?\);'
)
FULL_FENCE = re.compile(
    r'\b(?:asm|__asm__)\s+(?:volatile|__volatile__)\s*\(\s*'
    r'"sfence\.vma(?:\s+(?:zero|x0)\s*,\s*(?:zero|x0))?"\s*'
    r'(?::\s*:\s*:\s*"memory"\s*)?\);'
)


class StageError(Exception):
    pass


class Result(NamedTuple):
    code: Optional[int]
    output: str
    issue: str = ""


def show(layer, case, status, detail=""):
    print(f"[{layer}][{case}] {status}{'：' + detail if detail else ''}", flush=True)


def show_log(output):
    if output.strip():
        print(output.rstrip(), flush=True)


def select_program(env_var, *defaults):
    names = (os.environ[env_var],) if env_var in os.environ else defaults
    for name in names:
        found = shutil.which(name)
        if found:
            return os.path.abspath(found)
    raise StageError(f"找不到 {env_var} 对应的程序：{', '.join(names)}")


def run(command, work, seconds):
    env = {**os.environ, "LC_ALL": "C", "TMPDIR": str(work), "TMP": str(work), "TEMP": str(work)}
    try:
        proc = subprocess.run(command, cwd=work, env=env, stdin=subprocess.DEVNULL,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              encoding="utf-8", errors="replace", timeout=seconds)
        return Result(proc.returncode, proc.stdout)
    except subprocess.TimeoutExpired as error:
        output = (error.output or b"").decode("utf-8", errors="replace")
        return Result(None, output, f"超时（{seconds:g} 秒）")
    except OSError as error:
        return Result(None, "", f"无法执行 {command[0]}：{error}")


def build(command, work, phase):
    result = run(command, work, 60)
    if result.code == 0:
        return
    reason = result.issue or f"退出码 {result.code}"
    missing = re.findall(
        r"(?:undefined reference to\s+[`']|undefined symbol:\s+|^\s*\"_)"
        r"(page_insert|page_remove)(?:['\"\s]|$)", result.output, re.MULTILINE,
    )
    if missing:
        reason += f"；实际 pmm.c 未提供可链接的 {', '.join(sorted(set(missing)))} 定义"
    raise StageError(f"{phase}失败（{reason}）\n{result.output.rstrip()}".rstrip())


def host_sources(work):
    for origin in (PMM, PMM.with_suffix(".h")):
        source = origin.read_text(encoding="utf-8")
        source = PAGE_FENCE.sub(r"vmem_host_sfence_page(\1);", source)
        source = FULL_FENCE.sub("vmem_host_sfence_all();", source)
        (work / origin.name).write_text(
            f"#line 1 {json.dumps(str(origin), ensure_ascii=False)}\n{source}", encoding="utf-8"
        )


def build_host(work, test_source):
    compiler = select_program("CC", "cc")
    host_sources(work)
    host_io = work / "host_io.o"
    binary = work / "test"
    build([compiler, "-std=gnu11", "-c", TEST / "vmem_host.c", "-o", host_io], work, "宿主运行时编译")
    dead_strip = "-Wl,-dead_strip" if sys.platform == "darwin" else "-Wl,--gc-sections"
    includes = [f"-I{path}" for path in (work, *HEADERS)]
    build([compiler, *C_FLAGS, "-U__riscv", "-D__riscv_xlen=64", "-include", TEST / "vmem_host.h",
           *includes, work / "pmm.c", TEST / "vmem_fixture.c", TEST / test_source,
           host_io, dead_strip, "-o", binary], work, "宿主编译/链接")
    return binary


def build_guest(work):
    compiler = select_program("RISCV_CC", "riscv64-unknown-elf-gcc", "riscv64-linux-gnu-gcc")
    binary = work / "vmem.elf"
    includes = [f"-I{path}" for path in HEADERS]
    build([compiler, *C_FLAGS, "-DVMEM_GUEST", "-march=rv64imac_zicsr_zifencei", "-mabi=lp64",
           "-mcmodel=medany", "-msmall-data-limit=0", "-mno-relax", "-fno-pie", "-nostdlib", "-static",
           *includes, "-T", TEST / "vmem_guest.ld", "-Wl,--gc-sections,--build-id=none", "-o", binary,
           TEST / "vmem_start.S", TEST / "vmem_guest.c", TEST / "vmem_fixture.c", PMM,
           LAB / "kern/driver/intr.c", LAB / "libs/string.c", LAB / "libs/printfmt.c"], work, "RV64 编译/链接")
    return binary


def skip_cases(layer, cases, error):
    detail, _, log = str(error).partition("\n")
    show(layer, "准备", FAIL, detail)
    show_log(log)
    for case in cases:
        show(layer, case, SKIP, "依赖或构建未就绪")
    return SKIP


def native_layer(work, layer, cases, source):
    try:
        work.mkdir()
        binary = build_host(work, source)
    except (StageError, OSError) as error:
        return skip_cases(layer, cases, error)

    status = PASS
    for case in cases:
        result = run([binary, case], work, 15)
        detail = ""
        if layer == MODEL:
            marker = f"VMEM_MODEL_DONE seed={case} steps=256"
            detail = "256 步" if result.output.splitlines().count(marker) == 1 else ""
        if result.code == 0 and (layer != MODEL or detail):
            show(layer, case, PASS, detail)
        else:
            status = FAIL
            reason = result.issue or (f"用例退出码 {result.code}" if result.code != 0 else "缺少 256 步结束标识")
            show(layer, case, FAIL, reason)
            show_log(result.output)
    return status


def guest_layer(work):
    try:
        qemu = select_program("QEMU", "qemu-system-riscv64")
        timeout = float(os.environ.get("VMEM_QEMU_TIMEOUT", "20"))
        if not math.isfinite(timeout) or timeout <= 0:
            raise ValueError()
        work.mkdir()
        binary = build_guest(work)
    except ValueError:
        return skip_cases(GUEST, GUEST_CASES, "VMEM_QEMU_TIMEOUT 必须是正数（秒）")
    except (StageError, OSError) as error:
        return skip_cases(GUEST, GUEST_CASES, error)

    result = run([qemu, "-machine", "virt", "-accel", "tcg", "-m", "128M", "-smp", "1",
                  "-bios", "default", "-display", "none", "-monitor", "none", "-serial", "stdio",
                  "-no-reboot", "-kernel", binary], work, timeout)
    lines = result.output.splitlines()
    booted = lines.count("VMEM_BOOT") == 1
    done = [line for line in lines if line.startswith("VMEM_DONE ")]
    passed = True
    for case in GUEST_CASES:
        prefix = f"VMEM_CASE {case} "
        records = [line for line in lines if line.startswith(prefix)]
        if not booted or f"VMEM_BEGIN {case}" not in lines:
            show(GUEST, case, SKIP, "未观察到用例开始标识")
        elif records == [prefix + "PASS"]:
            traps = [line.removeprefix("VMEM_TRAP ") for line in lines if line.startswith("VMEM_TRAP ")]
            show(GUEST, case, PASS, "；".join(traps) if case == "protect-and-revoke" else "")
            continue
        else:
            detail = ", ".join(records) if records else (result.issue or "缺少用例结束标识")
            show(GUEST, case, FAIL, detail)
        passed = False

    problem = result.issue or (f"QEMU 退出码 {result.code}" if result.code != 0 else "")
    if not problem and not booted:
        problem = "未观察到唯一的 VMEM_BOOT 启动标识"
    if not problem and done != ["VMEM_DONE PASS"]:
        problem = ", ".join(done) if done else "未观察到 VMEM_DONE 结束标识"
    if problem:
        show(GUEST, "启动及结束", FAIL, problem)
    elif passed:
        show(GUEST, "启动及结束", PASS, "VMEM_BOOT → VMEM_DONE PASS")
    if problem or not passed:
        show_log(result.output)
        return FAIL
    return PASS


def summary(unit, guest, model):
    print(f"汇总：{UNIT}={unit}；{GUEST}={guest}；{MODEL}={model}", flush=True)
    return 0 if unit == guest == model == PASS else 1


def main():
    if len(sys.argv) != 1:
        show("运行器", "参数", FAIL, "此入口不接受参数")
        return summary(SKIP, SKIP, SKIP)
    try:
        with tempfile.TemporaryDirectory(prefix=".vmem-", dir=TEST) as temp:
            work = Path(temp)
            unit = native_layer(work / "unit", UNIT, UNIT_CASES, "vmem_unit.c")
            guest = guest_layer(work / "guest")
            model = native_layer(work / "model", MODEL, SEEDS, "vmem_model.c")
    except OSError as error:
        show("运行器", "临时目录", FAIL, str(error))
        return summary(SKIP, SKIP, SKIP)
    return summary(unit, guest, model)


if __name__ == "__main__":
    sys.exit(main())
