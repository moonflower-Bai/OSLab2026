# Proposal

## Why

Lab5 骨架缺少 Lab2–Lab4 的物理内存、异常处理与进程管理实现。Lab5 同时需要从同一个陷阱帧恢复到内核态或用户态，以便用户态再次陷入时能切回内核栈。

## What Changes

- 以 `origin/Dev/Lab4/main` 的集成实现为迁移基线，核对 Lab2 和 Lab3 稳定分支，按 Lab5 现有接口迁入物理页分配、页表操作、陷阱分发、内核线程创建、上下文切换和调度。
- 完成 Lab5 的 `RESTORE_ALL`，使硬件陷阱与 `forkrets` 构造的陷阱帧共享返回路径。
- Lab5 本章独立练习（例如 ELF 装载、系统调用和用户进程生命周期）沿用课程范围。本次验收用独立的最小 U 态程序测试陷阱往返。
- 实验源文件限于 Lab5。当前章切换需先获得授权，冻结列表沿用当前配置。

## Capabilities

### New Capabilities

- `lab5-prerequisites` — Lab5 继承的内存与内核进程行为，以及 S/U 态陷阱上下文往返。

### Modified Capabilities

无。

## Impact

涉及 Lab5 的内存管理、进程、调度和陷阱代码。验证使用 RISC-V GCC、binutils 和 QEMU virt。
