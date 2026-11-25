# Design

## Context

当前分支为 `Dev/Lab5/main`，工作区已有未跟踪的 `lab5/` 骨架。当前章节仍为 `lab4`，冻结列表为空。须经授权切换当前章节后才能修改 Lab5 实验文件。集成参考为 `origin/Dev/Lab4/main` 的 `d4bb252`，其中 `b02fba3` 补齐了 Lab3 陷阱和 Lab4 进程管理。

## Goals / Non-Goals

恢复骨架中缺失的前置内核能力，并允许从硬件陷阱和人工构造的进程启动帧返回到 S 态或 U 态。工作范围限于继承功能和异常返回。

## Decisions

- 以函数为单位核对 Lab2 分配器和 Lab4 内存、进程及调度接口。保留 Lab5 已有的进程链表、用户页表接口、陷阱分发外层与 36 槽陷阱帧布局。
- 向 `trap()` 传递帧指针。返回前按帧内 `sstatus.SPP` 设置 `sscratch`：S 态置零，U 态设为当前陷阱帧末端。保持 `SIE` 清零，由 `sret` 从 `SPIE` 恢复中断状态。恢复 `sepc`、`stval` 与通用寄存器，最后从帧恢复 `sp`。`scause` 由陷阱帧保留供分发和诊断。
- 返回路径只从内核栈读取陷阱帧。U 态的栈指针只作为待恢复值。不得为了沿用 Lab3 的异常指令长度判定而直接解引用用户 `sepc`。
- 在 QEMU 使用 S 态测试及最小 U 态测试驱动，覆盖人工构造帧进入 U 态、再次陷入、嵌套 S 态陷阱、以及返回后保存寄存器和栈的比对。

## Risks / Trade-offs

Lab5 独立练习目前存在空缺，完整评分依赖这些练习。用隔离的测试副本驱动前置能力和异常返回，单独记录原始骨架构建中由独立练习造成的阻塞。

返回语义参考 [RISC-V Supervisor-Level ISA](https://docs.riscv.org/reference/isa/v20260120/priv/supervisor.html)。
