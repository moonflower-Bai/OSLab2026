## Context

`lab3/kern/trap/trap.c` 是官方骨架，`interrupt_handler()` 和 `exception_handler()` 均未实现。本分支只实现中断处理部分；功能模块一和异常处理由组内其他成员负责。官方骨架没有定义 `SHUTDOWN_AFTER`，需要在本 change 中补充。

仓库内其他依赖（lab2 `page_init`、`best_fit_pmm.c` 的 free area、模块一上下文恢复）不进入本 change；验证时只在仓库外临时副本中补齐。

## Goals / Non-Goals

**Goals:**
- 让时钟中断持续触发并完成计数、输出和最终关机。
- 对软件中断、外部中断和未知中断给出可观察的区分输出。
- 保持与 `trap_dispatch()`、`trap()`、`print_trapframe()` 的既有接口一致。

**Non-Goals:**
- 不实现 `trapentry.S` 的上下文恢复/入口。
- 不实现 `exception_handler()`。
- 不提交 lab2 依赖或工具链兼容修改。

## Decisions

- **中断原因先屏蔽最高位**：`scause` 最高位为 1 表示中断；`IRQ_S_*` 常量本身不含该位。`interrupt_handler()` 先用 `tf->cause & ~((uintptr_t)1 << 63)` 得到原因号，再 `switch`。
- **`SHUTDOWN_AFTER` 取 10**：按 `ticks / TICK_NUM` 统计 `print_ticks()` 的累计输出次数；10 次对应 1000 个 tick（约 10 秒）。Test 3 只检查到 200 tick，因此不会提前关机；关机路径另做长跑验证。
- **未知中断同时输出原因和 trapframe**：`cprintf("Unknown interrupt: cause %d\n", ...)` 满足 Test 2 的检查，`print_trapframe(tf)` 保留指导书要求的兜底调试信息。
- **只加 `exception_handler()` 声明**：`trap_dispatch()` 在 C99 `-Werror` 下需要可见声明；本分支不提供定义。
- **仓库外验证**：临时副本里补 lab2 `page_init`、`best_fit` free area、模块一 `RESTORE_ALL`/`__alltraps` 和 `exception_handler` 链接桩；用支持 `sbadaddr` 的 binutils 2.34 包装工具链构建，用 `-kernel` 跑 QEMU。所有临时修改不提交。

## Risks / Trade-offs

- 仓库内 `make -C lab3` 仍会因 `sbadaddr`、lab2 `page_init`、`best_fit` 和 `exception_handler` 缺失而失败；每个 commit 只保证 `trap.o` 编译通过。
- 官方 Test 3 只验证 200 tick；另做长跑确认第 10 次 `100 ticks` 后 `sbi_shutdown()`。
- 旧汇编器包装需要 `-mno-arch-attr` 并过滤新 binutils 参数；若失败，临时副本把 `sbadaddr` 改成 `stval`，不提交。
- `SHUTDOWN_AFTER=10` 是指导书未指定的取值；如组内约定不同，只改该宏。
