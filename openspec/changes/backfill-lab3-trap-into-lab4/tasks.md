# Tasks

## 1. 记录与规格

- [x] 1.1 创建 `backfill-lab3-trap-into-lab4` 的 proposal、design、spec、tasks 和 prompt 证据。
- [x] 1.2 运行严格校验确保 OpenSpec 产物合规。

## 2. 迁移 Lab3 Trap 实现

- [x] 2.1 在 `lab4/kern/trap/trapentry.S` 补齐 `RESTORE_ALL` 与 `__alltraps` 调用。
- [x] 2.2 在 `lab4/kern/trap/trap.c` 补齐 `trap_in_kernel`、`exception_instruction_length`、`exception_handler` 与 `interrupt_handler`。

## 3. 验证与提交

- [x] 3.1 编译 `trapentry.o` 和 `trap.o`，验证迁移代码语法和符号。
- [x] 3.2 运行内核编译及测试，验证正常运行。
- [x] 3.3 提交本次迁移修改。
