# Design

## Context

Lab 4 骨架在 `proc.c` 和 `sched.c` 的 `//LAB4 function3` 标记处留空，`switch.S` 只有一行标记。模块二的 `proc_init` 会创建 `idleproc` 和 `initproc`，并把 `idleproc->need_resched` 置为 1；`kern_init` 最后调用 `cpu_idle`。模块三需要沿 `cpu_idle -> schedule -> proc_run -> switch_to` 完成第一次切换，并让后续线程按 Round-Robin 轮转。

当前分支缺少模块一/二和 Lab2/Lab3 代码，无法独立完整链接；本 change 只提交模块三的四个函数。

## Goals / Non-Goals

**Goals:**

- 在 `proc_list` 中选出下一个 `PROC_RUNNABLE` 线程；没有可运行线程时回退 `idleproc`。
- 在关中断临界区内完成 `current` 更新、`satp` 切换和上下文切换。
- 让 `switch_to` 严格按 `struct context` 布局保存/恢复 14 个寄存器。
- 让 `cpu_idle` 在 `need_resched` 时触发调度。

**Non-Goals:**

- 不实现 `proc_init`、`kernel_thread`、`do_fork` 等模块二函数。
- 不实现 `page_insert`、`page_remove` 等模块一函数。
- 不实现 Lab2/Lab3 的物理内存、trap 入口和异常/中断处理。
- 不实现用户进程、ELF 加载、定时器抢占调度或 `do_exit`。
- 不修改 `lsatp`、测试、Makefile 或评分脚本。

## Decisions

- **Round-Robin 起点**：当前不是 `idleproc` 时从 `current->list_link` 的下一个位置开始；当前是 `idleproc` 时从 `&proc_list` 开始。`proc_list` 是带哨兵的循环链表，遍历必须回绕一整圈。
- **回退**：跳过 `&proc_list` 哨兵；整圈没有 `PROC_RUNNABLE` 时选择 `idleproc`。不直接运行 `PROC_SLEEPING`、`PROC_UNINIT` 或 `PROC_ZOMBIE`。
- **统计**：选中 `next` 后执行 `next->runs++`，`proc_run` 不重复计数；`test_lab4_process_create` 在调度前断言 `idleproc->runs == 0`。
- **原子性**：`schedule` 和 `proc_run` 都使用 `local_intr_save/restore`。`current`、`satp`、`switch_to` 必须在同一临界区，避免中断看到不一致状态。
- **satp**：骨架的 `lsatp(unsigned int)` 使用 `SATP32_MODE`，在 RV64/Sv39 下会把 MODE 写成 0（Bare）。本 change 不修改共享头文件，在 `proc_run` 内按 `pmm.c` 的启动写法直接写 `0x8000000000000000 | (pgdir >> 12)`；页表变化时执行 `sfence.vma`。
- **上下文布局**：`struct context` 有 14 个 `uintptr_t`。`switch_to` 只保存 `ra`、`sp`、`s0`~`s11`，不保存 caller-saved 寄存器；`sp` 最后恢复。
- **首次运行**：新线程的 `context.ra` 由模块二设为 `forkret`，`context.sp` 指向 trapframe；`switch_to` 返回后走 `forkret -> forkrets -> __trapret -> kernel_thread_entry`。该链路不在本 change 修改。
- **验证**：仓库内只做对象级编译；仓库外以队友 Lab4 模块一/二分支为基线，临时补最小 Lab3 返回链后运行官方 `check Schedule`。

## Risks / Trade-offs

- [模块二尚未合并] → 本分支只验证目标文件；完整启动在仓库外组合验证。
- [Lab3 返回链缺失] → 临时 shim 的 `RESTORE_ALL` 必须从 trapframe 的 `sp` 字段恢复，不能用固定 `addi` 代替。
- [`satp` 同值] → Lab4 内核线程共享 `boot_pgdir_pa`，仍按统一路径写 `satp`；`sfence.vma` 只在页表变化时执行。
- [链表顺序] → 使用 `last` 和 `do/while` 回绕，避免漏掉当前进程之前的就绪线程。
