# Proposal

## Why

Lab 4 的功能模块三负责把已经创建出来的内核线程真正交给 CPU。当前骨架中 `cpu_idle`、`schedule`、`proc_run` 和 `switch_to` 均为空，`initproc` 无法从 `idleproc` 手中接管 CPU，后续内核线程也无法轮转执行。

## What Changes

- 在 `kern/process/proc.c` 实现 `proc_run` 和 `cpu_idle`。
- 在 `kern/schedule/sched.c` 实现 `schedule`，从 `proc_list` 中选择 `PROC_RUNNABLE` 线程，支持 Round-Robin 起点、链表回绕和 `idleproc` 回退。
- 在 `kern/process/switch.S` 实现 `switch_to`，只保存和恢复 `ra`、`sp`、`s0`~`s11`。
- 在 `proc_run` 内直接写 Sv39 的 `satp`，不修改 `libs/riscv.h` 中仅适用于 RV32 的 `lsatp`。
- 在仓库外临时组合模块一/二和最小 Lab3 返回链，运行官方 `check Schedule`；所有临时依赖不进入提交。

## Capabilities

### New Capabilities

- `lab4-process-scheduling`: 内核线程的调度选择、进程切换和空闲循环。

### Modified Capabilities

无。

## Impact

涉及 `lab4/lab4_new/kern/process/proc.c`、`lab4/lab4_new/kern/process/switch.S`、`lab4/lab4_new/kern/schedule/sched.c` 和本 change。依赖模块二创建的 `idleproc`/`initproc`、`proc_list`、`context` 和首次运行恢复链；依赖模块一/Lab2 的物理内存和 page table 能力。模块一/二、Lab2/Lab3、测试、Makefile、评分脚本和 `libs/riscv.h` 不在本次修改范围内。
