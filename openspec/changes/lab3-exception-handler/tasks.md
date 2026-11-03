# Tasks

## 1. 记录

- [x] 1.1 将本需求的用户原文写入 `openspec/changes/lab3-exception-handler/prompt.md`，并确认其中没有敏感值。验证：该文件含 `reviewed_for_sensitive_data: true` 与原文标题。

## 2. 异常处理

- [x] 2.1 在 `lab3/kern/trap/trap.c` 实现 `exception_handler`：cause 2 与 cause 3 分别打印 `Illegal instruction at` 和 `Breakpoint at`，地址使用 `0x%016llx`，并只在这两个分支按第一条 16 位半字的低两位推进 `epc`；其余 cause 调用 `print_trapframe`，再按 `trap_in_kernel(tf)` 决定是否 `panic`。不改 `interrupt_handler`。验证：`exception_handler` 含上述两个专属字符串和 `trap_in_kernel`，`interrupt_handler` 仍只有空体。

- [x] 2.2 在 `lab3/kern/trap/trapentry.S` 的 `__alltraps` 中，于 `SAVE_ALL` 之后用 `mv a0, sp` 和 `jal trap` 进入 `trap()`，不修改 `SAVE_ALL` 与 `RESTORE_ALL` 宏体，不按 cause 分流，并去掉该缺口中的练习占位。验证：`__alltraps` 含 `jal trap`，两个宏体与实现前一致，该缺口不含 `TODO`。

## 3. 校验

- [x] 3.1 运行 `openspec validate lab3-exception-handler --strict`。验证：命令退出码为 0。

- [x] 3.2 运行 `bash lab3/test/exception.sh`。外部前提：系统自带 C 编译器，以及 `riscv64-unknown-elf-gcc`、`ld`、`objcopy`。验证：命令退出码为 0，输出中没有「错误」，宿主机各项为「通过」，末行为「QEMU层仅链接检查」。不读取 `lab3/test/exception/` 的源码。
