## ADDED Requirements

### Requirement: 调度选择

系统 SHALL 在 `schedule` 中清除当前进程的 `need_resched`，从 `proc_list` 中选择下一个处于 `PROC_RUNNABLE` 状态的进程，并在找不到可运行进程时回退到 `idleproc`。

#### Scenario: 从 idleproc 首次调度

- **WHEN** `current` 是 `idleproc`，且 `proc_list` 中存在 `PROC_RUNNABLE` 的 `initproc`
- **THEN** `schedule` 选择 `initproc` 并调用 `proc_run`

#### Scenario: 回绕查找

- **WHEN** 当前进程不是 `idleproc`，且其后的链表节点都不可运行，而其前面的节点可运行
- **THEN** `schedule` 从当前进程的下一个节点开始，绕回链表头部继续查找

#### Scenario: 没有可运行进程

- **WHEN** 遍历完整圈后没有 `PROC_RUNNABLE` 进程
- **THEN** `schedule` 选择 `idleproc` 作为 `next`

#### Scenario: 不选择非就绪进程

- **WHEN** 进程状态为 `PROC_UNINIT`、`PROC_SLEEPING` 或 `PROC_ZOMBIE`
- **THEN** `schedule` 不将其作为 `next`

### Requirement: 调度原子性

`schedule` SHALL 在关中断的临界区内完成 `need_resched` 清除、链表选择和 `proc_run` 调用；嵌套的 `local_intr_save/restore` 不得提前开启中断。

#### Scenario: 嵌套临界区

- **WHEN** `schedule` 已关闭中断并调用 `proc_run`
- **THEN** `proc_run` 恢复时保持中断关闭，直到 `schedule` 恢复原中断状态

### Requirement: 进程切换

`proc_run` SHALL 在目标进程不同于当前进程时，于临界区内更新 `current`、切换到目标进程的页表并调用 `switch_to`。

#### Scenario: 切换到新进程

- **WHEN** `proc_run(proc)` 收到不同于 `current` 的目标进程
- **THEN** 在 `switch_to` 前将 `current` 更新为目标进程，并把目标进程的 `pgdir` 写入 `satp`

#### Scenario: 目标就是当前进程

- **WHEN** `proc_run` 的目标等于 `current`
- **THEN** 不执行上下文切换

### Requirement: 上下文保存与恢复

`switch_to` SHALL 按照 `struct context` 的布局保存 `from` 的 `ra`、`sp`、`s0`~`s11`，并恢复 `to` 的同组寄存器；不得保存或恢复 caller-saved 寄存器。

#### Scenario: 首次进入新线程

- **WHEN** `to->context.ra` 为 `forkret` 且 `to->context.sp` 指向新线程的 trapframe
- **THEN** `switch_to` 恢复这些寄存器后返回到 `forkret`

### Requirement: 空闲循环

`cpu_idle` SHALL 持续循环，并仅在 `current->need_resched` 非零时调用 `schedule`。

#### Scenario: 空闲线程让出 CPU

- **WHEN** `cpu_idle` 运行时 `current->need_resched` 为 1
- **THEN** 调用 `schedule` 触发调度

#### Scenario: 没有调度需求

- **WHEN** `current->need_resched` 为 0
- **THEN** `cpu_idle` 继续循环，不调用 `schedule`
