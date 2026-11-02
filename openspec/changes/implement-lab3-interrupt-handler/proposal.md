## Why

Lab3 的功能模块二要求实现中断与异常处理。本分支只负责其中的中断处理部分：当前 `lab3/kern/trap/trap.c` 仍缺少 `interrupt_handler()`，时钟中断收到一次后不会重新设置下一次事件，其他中断也没有分类输出。

## What Changes

- 在 `lab3/kern/trap/trap.c` 中实现 `interrupt_handler(struct trapframe *tf)`：
  - `IRQ_S_TIMER`：调用 `clock_set_next_event()` 重新设置下一次时钟事件，递增 `ticks`；每 `TICK_NUM` 次输出 `100 ticks`；累计输出达到 `SHUTDOWN_AFTER` 次后调用 `sbi_shutdown()`。
  - `IRQ_S_SOFT`、`IRQ_S_EXT`：输出对应提示。
  - 未知中断：输出 `Unknown interrupt: cause <n>` 并调用 `print_trapframe(tf)`。
- 增加 `SHUTDOWN_AFTER` 定义、`<sbi.h>` 引用，以及 `exception_handler()` 的函数声明（不实现）。

## Non-goals

- 不实现 `kern/trap/trapentry.S` 的 `RESTORE_ALL` / `__alltraps`（功能模块一）。
- 不实现 `exception_handler()` 的异常处理逻辑。
- 不修改 `trap_dispatch()`、`trap()`、`print_trapframe()` 等既有函数。
- 不把 lab2 的 `page_init` / best-fit 代码提交到仓库；它们只在仓库外临时副本中用于 Test 3 验证。
- 不修改 `sbadaddr` 的官方写法；本地用旧汇编器包装验证。
- 不修改评分脚本、Makefile、测试代码或其他章节目录。
