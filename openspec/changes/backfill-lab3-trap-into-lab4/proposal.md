# Proposal

## Why

Lab4 骨架中的 Trap 上下文恢复、入口调用、异常处理与中断处理留有 Lab3 练习占位，无法正常编译和启动。Lab4 的内核线程管理与后续运行依赖 Lab3 已经验证的 Trap 处理机制，因此需要将已完成的 Lab3 代码迁入 Lab4。

## What Changes

- 在 `lab4/kern/trap/trapentry.S` 中补齐 `RESTORE_ALL` 宏定义，恢复通用寄存器、特权 CSR 和内核栈。
- 在 `lab4/kern/trap/trapentry.S` 中补齐 `__alltraps`，通过 `move a0, sp` 和 `jal trap` 进入 C 处理函数。
- 在 `lab4/kern/trap/trap.c` 中迁入 `trap_in_kernel`、`exception_instruction_length`、`exception_handler` 和 `interrupt_handler`。
- 保留 Lab4 进程调度所需的 `forkrets` 符号及既有接口。

## Capabilities

### New Capabilities

- `lab4-trap-backfill`: 在 Lab4 中提供源自 Lab3 的 Trap 上下文恢复、异常分派与时钟中断处理能力。

### Modified Capabilities

无。

## Impact

涉及 `lab4/kern/trap/trapentry.S`、`lab4/kern/trap/trap.c` 和本 change。不修改 Lab4 现有进程与内存管理代码。
