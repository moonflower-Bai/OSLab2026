# Spec Delta

## Purpose

在 Lab4 中迁入 Lab3 已验证的 Trap 上下文恢复、异常分派与时钟中断处理，为 Lab4 内核提供完整的 Trap 处理和时钟事件支持。

## ADDED Requirements

### Requirement: Trap 上下文恢复与处理入口

Lab4 的 `kern/trap/trapentry.S` MUST 提供完整的通用寄存器与特权 CSR 恢复宏 `RESTORE_ALL`，并在 `__alltraps` 中将 `sp` 传递给 `trap()` 函数。

#### Scenario: 从 Trap 中返回
- **WHEN** CPU 完成异常或中断处理并从 `trap()` 返回
- **THEN** 系统按槽位恢复特权 CSR 及全部通用寄存器，最后恢复内核栈指针并通过 `sret` 返回被打断的上下文。

### Requirement: 异常分派与时钟中断处理

Lab4 的 `kern/trap/trap.c` MUST 正确分派断点、非法指令等同步异常，并对时钟中断进行重设与计数。

#### Scenario: 发生时钟中断
- **WHEN** 定时器产生中断 `IRQ_S_TIMER`
- **THEN** 处理程序重设下一次时钟事件并递增 `ticks` 计数器，每 100 个 tick 打印一次提示。
