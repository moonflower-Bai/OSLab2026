# Design

## Context

Lab4 的 `kern/trap/trapentry.S` 和 `kern/trap/trap.c` 包含 `#lab3:填写你在lab3中实现的代码` 标记，导致编译器报告 `interrupt_handler` 和 `exception_handler` 未声明。Lab3 已经在 `Lab3` 分支中完成了完整的上下文恢复、异常指令步进以及基于时钟的中断处理。

## Goals / Non-Goals

**Goals:**
- 将 Lab3 的 `RESTORE_ALL` 和 `__alltraps` 迁入 `lab4/kern/trap/trapentry.S`。
- 将 Lab3 的 `exception_handler` 和 `interrupt_handler` 迁入 `lab4/kern/trap/trap.c`。
- 确保 `trapentry.o` 和 `trap.o` 编译通过。

**Non-Goals:**
- 不修改 `forkrets` 汇编逻辑。
- 不引入 Lab3 的关机退出桩（保留 Lab4 的长周期运行约定）。

## Decisions

- **RESTORE_ALL 顺序**：先恢复 `sstatus`、`sepc`、`stval`，再恢复 `x1`、`x3`~`x31`，最后从槽位 2 恢复 `sp`。
- **异常推进**：在 `exception_handler` 中根据指令首半字低 2 位识别 16/32 位指令长度并推进 `epc`。
- **时钟中断**：在 `interrupt_handler` 中调用 `clock_set_next_event()`，递增 `ticks`，每 100 个 tick 调用 `print_ticks()`。

## Risks / Trade-offs

- Lab3 的中断处理在独立验证时包含关机逻辑；Lab4 作为多线程运行环境不应提前关机。因此在 `interrupt_handler` 中保留周期性 `print_ticks()` 计数，避免过早中断调度。
