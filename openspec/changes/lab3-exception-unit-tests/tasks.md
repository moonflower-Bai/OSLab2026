# Tasks

## 1. 记录

- [x] 1.1 将本需求的用户原文写入 `openspec/changes/lab3-exception-unit-tests/prompt.md`，并确认其中没有敏感值。验证：该文件含 `reviewed_for_sensitive_data: true` 与原文标题。

## 2. 宿主机测试

- [x] 2.1 新增 `lab3/test/exception/` 下的宿主机运行时与用例，桩住 `cprintf`、`__panic` 和 `interrupt_handler`，经 `trap()` 检查断点、非法指令、指令长度、未知异常和 trapframe 字段。验证：源码中不存在恒真断言，且失败路径只拼接当前值。
- [x] 2.2 新增 `lab3/test/exception.sh`，用宿主机编译器链接 `lab3/kern/trap/trap.c` 并运行宿主机测试，同时交叉编译并链接 QEMU 层。外部前提：系统自带 C 编译器，以及 `riscv64-unknown-elf-gcc`、`ld`、`objcopy`。验证：`bash lab3/test/exception.sh` 可执行；当前空的 `exception_handler` 使宿主机结果为「错误」且退出码非零；链接成功时输出「通过 QEMU链接」。

## 3. QEMU 测试源码

- [x] 3.1 在 `lab3/test/exception/` 编写 QEMU 层保存/恢复与 S 态 `ebreak` 委托测试，检查保存的 `x0` 为 0，并在进入 `trap()` 时核对 trapframe 槽位。不读取 `medeleg`。`exception.sh` 先跑宿主机测试，再用 `--wrap=trap --wrap=kern_init` 做链接检查；找不到 `riscv64-unknown-elf-gcc` 时跳过且不因此失败。验证：输出「QEMU层仅链接检查」，且不设置 `EXCEPTION_RUN_QEMU` 时不启动 `qemu-system-riscv64`。

## 4. 对抗与回归

- [x] 4.1 在临时目录放置一份符合规格的 `exception_handler`，用 `EXCEPTION_EXTRA_SRC` 运行脚本。验证：`EXCEPTION_EXTRA_SRC=<临时正确实现> bash lab3/test/exception.sh` 退出码为 0，且输出均为「通过」。
- [x] 4.2 让不读取 `lab3/test/` 的子代理在临时目录生成错误 `exception_handler` 变体，并逐个用 `EXCEPTION_EXTRA_SRC` 运行。验证：每个变体至少有一项输出「错误」且退出码非零；变体路径不在仓库内。

## 5. 校验

- [x] 5.1 运行 `openspec validate lab3-exception-unit-tests --strict`。验证：命令退出码为 0。
