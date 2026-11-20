# Proposal

## Why

Lab 4 骨架尚不能创建内核线程；第二个功能模块需要建立进程管理与首次运行所需的现场。

## What Changes

- 将当前章节设为 lab4，使个人实验分支与章节状态一致。
- 实现 PCB 初始化、idleproc 和 initproc 初始化、内核线程包装及 fork 的资源管理。
- 增加隔离测试验证线程创建和失败清理，记录依赖其他模块才能执行的集成检查。
- 不实现虚拟内存页操作、调度和上下文切换，也不实现用户地址空间复制。

## Capabilities

### New Capabilities

- `lab4-process-init`: 内核线程创建与进程初始化。

### Modified Capabilities

无。

## Impact

涉及 `lab-status.json`、Lab 4 的 `kern/process/proc.c` 和本 change；依赖物理内存、调度模块和 RISC-V 工具链。
