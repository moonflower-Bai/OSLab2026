#!/usr/bin/env bash
# 宿主机异常测试，并交叉编译、链接 QEMU 层（ld --wrap=trap --wrap=kern_init）。
# 可选：EXCEPTION_EXTRA_SRC 指向额外的 exception_handler，只替换宿主机被测实现。
#
# 在完整内核上运行：把下面两个目标文件和内核一起链接
#   qemu_exception.o  qemu_exception_s.o
#   riscv64-unknown-elf-ld -m elf64lriscv -nostdlib \
#     --wrap=trap --wrap=kern_init -T lab3/tools/kernel.ld \
#     -o ucore.elf <内核目标文件> qemu_exception.o qemu_exception_s.o
#   riscv64-unknown-elf-objcopy ucore.elf --strip-all -O binary ucore.img
#   EXCEPTION_RUN_QEMU=1 EXCEPTION_QEMU_IMG=ucore.img bash lab3/test/exception.sh
# 或手动：
#   qemu-system-riscv64 -machine virt -nographic -bios default \
#     -device loader,file=ucore.img,addr=0x80200000
# 默认不启动 qemu-system-riscv64。当前 binutils 不接受 trapentry.S 里的 sbadaddr 时，完整镜像需要旧工具链。
set -euo pipefail

script_dir=$(cd "$(dirname "$0")" && pwd)
lab_dir=$(cd "$script_dir/.." && pwd)
work=$(mktemp -d)
log="$work/build.log"
trap 'rm -rf "$work"' EXIT

inc=(
    -I "$lab_dir/libs"
    -I "$lab_dir/kern/debug"
    -I "$lab_dir/kern/driver"
    -I "$lab_dir/kern/trap"
    -I "$lab_dir/kern/mm"
)
cflags=(
    -std=gnu99
    -Wall
    -D__riscv_xlen=64
    -fno-builtin
    -g
    -O0
)

show_build_error() {
    local kind="$1"
    local line
    line=$(tail -n 1 "$log")
    echo "错误 ${kind}: ${line}"
    cat "$log"
    exit 1
}

build_host() {
    if ! cc "${cflags[@]}" "${inc[@]}" \
        -include "$script_dir/exception/host_include/host_proto.h" \
        -include "$script_dir/exception/host_include/host_prelude.h" \
        -include "$script_dir/exception/host_include/host_csr.h" \
        -c "$lab_dir/kern/trap/trap.c" -o "$work/trap.o" >"$log" 2>&1; then
        show_build_error "编译"
    fi
    if ! cc "${cflags[@]}" -c "$script_dir/exception/host_runtime.c" \
        -o "$work/runtime.o" >"$log" 2>&1; then
        show_build_error "编译"
    fi
    if ! cc "${cflags[@]}" "${inc[@]}" -c "$script_dir/exception/host_test.c" \
        -o "$work/test.o" >"$log" 2>&1; then
        show_build_error "编译"
    fi
    local objs=("$work/trap.o" "$work/runtime.o" "$work/test.o")
    if [[ -n "${EXCEPTION_EXTRA_SRC:-}" ]]; then
        if ! cc "${cflags[@]}" "${inc[@]}" \
            -include "$script_dir/exception/host_include/host_csr.h" \
            -c "${EXCEPTION_EXTRA_SRC}" -o "$work/extra.o" >"$log" 2>&1; then
            show_build_error "编译"
        fi
        objs+=("$work/extra.o")
    fi
    if ! cc -o "$work/host_test" "${objs[@]}" >"$log" 2>&1; then
        show_build_error "链接"
    fi
}

build_qemu() {
    local cc=riscv64-unknown-elf-gcc
    local ld=riscv64-unknown-elf-ld
    if ! command -v "$cc" >/dev/null 2>&1 || ! command -v "$ld" >/dev/null 2>&1; then
        echo "未找到 riscv64-unknown-elf-gcc，跳过 QEMU 层"
        return 0
    fi
    local qinc=(
        -I "$lab_dir/libs"
        -I "$lab_dir/kern/debug"
        -I "$lab_dir/kern/driver"
        -I "$lab_dir/kern/trap"
        -I "$lab_dir/kern/mm"
    )
    if [[ -d "$lab_dir/kern/arch" ]]; then
        qinc+=(-I "$lab_dir/kern/arch")
    fi
    local qflags=(-mcmodel=medany -std=gnu99 -Wno-unused -Wall -Werror -fno-builtin -nostdinc
        -fno-stack-protector -g -O2)
    if ! "$cc" "${qflags[@]}" "${qinc[@]}" -c "$script_dir/exception/qemu_exception.c" \
        -o "$work/qemu_c.o" >"$log" 2>&1; then
        echo "错误 编译: $(tail -n 1 "$log")"
        cat "$log"
        return 1
    fi
    if ! "$cc" "${qflags[@]}" "${qinc[@]}" -c "$script_dir/exception/qemu_exception.S" \
        -o "$work/qemu_s.o" >"$log" 2>&1; then
        echo "错误 编译: $(tail -n 1 "$log")"
        cat "$log"
        return 1
    fi
    if ! "$cc" "${qflags[@]}" "${qinc[@]}" -c "$script_dir/exception/qemu_link_stubs.c" \
        -o "$work/qemu_stubs.o" >"$log" 2>&1; then
        echo "错误 编译: $(tail -n 1 "$log")"
        cat "$log"
        return 1
    fi
    if ! "$ld" -m elf64lriscv -nostdlib \
        --wrap=trap --wrap=kern_init \
        -o "$work/qemu_test.elf" \
        "$work/qemu_c.o" "$work/qemu_s.o" "$work/qemu_stubs.o" >"$log" 2>&1; then
        echo "错误 链接: $(tail -n 1 "$log")"
        cat "$log"
        return 1
    fi
    echo "QEMU层仅链接检查"
    return 0
}

run_qemu() {
    local img="${EXCEPTION_QEMU_IMG:-}"
    if [[ "${EXCEPTION_RUN_QEMU:-}" != 1 ]]; then
        return 0
    fi
    if [[ -z "$img" || ! -f "$img" ]]; then
        echo "错误 QEMU: 没有镜像"
        return 1
    fi
    if ! command -v qemu-system-riscv64 >/dev/null 2>&1; then
        echo "错误 工具: qemu-system-riscv64"
        return 1
    fi
    qemu-system-riscv64 -machine virt -nographic -bios default \
        -device "loader,file=${img},addr=0x80200000" \
        >"$work/qemu.out" 2>&1 &
    local qpid=$!
    local i
    for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
        if ! kill -0 "$qpid" 2>/dev/null; then
            break
        fi
        sleep 1
    done
    if kill -0 "$qpid" 2>/dev/null; then
        kill "$qpid" 2>/dev/null || true
        wait "$qpid" 2>/dev/null || true
        echo "错误 QEMU超时"
        cat "$work/qemu.out"
        return 1
    fi
    wait "$qpid" || true
    cat "$work/qemu.out"
    # 6 项陷阱环境 + 恢复后的 x0..x31 + 进入时 tf 的 x0..x31、cause、SPP。
    local expect=72
    local pass_n fail_n
    pass_n=$(grep -c '^通过 ' "$work/qemu.out" || true)
    fail_n=$(grep -c '^错误 ' "$work/qemu.out" || true)
    if [[ "$fail_n" -ne 0 || "$pass_n" -ne "$expect" ]]; then
        if [[ "$fail_n" -eq 0 ]]; then
            echo "错误 QEMU结果: ${pass_n}"
        fi
        return 1
    fi
    return 0
}

build_host
set +e
"$work/host_test"
host_ec=$?
build_qemu
qemu_ec=$?
if [[ "$qemu_ec" -eq 0 ]]; then
    run_qemu
    qemu_ec=$?
fi
set -e
if [[ "$host_ec" -ne 0 || "$qemu_ec" -ne 0 ]]; then
    exit 1
fi
