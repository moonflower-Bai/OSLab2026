## ADDED Requirements

### Requirement: Timer interrupts are re-armed and counted
`interrupt_handler()` MUST 在收到 `IRQ_S_TIMER` 时调用 `clock_set_next_event()` 设置下一次时钟事件，并使 `ticks` 递增。每当 `ticks` 达到 `TICK_NUM` 的整数倍时，MUST 输出一次 `100 ticks`；累计输出达到 `SHUTDOWN_AFTER` 次后 MUST 调用 `sbi_shutdown()`。

#### Scenario: Consecutive timer interrupts
- **WHEN** 内核使能时钟中断并持续运行
- **THEN** 每 100 个 tick 输出一行 `100 ticks`，且时钟中断持续发生而不是只发生一次

#### Scenario: Shutdown threshold
- **WHEN** `print_ticks()` 的累计输出次数达到 `SHUTDOWN_AFTER`
- **THEN** 调用 `sbi_shutdown()` 结束系统

### Requirement: Interrupt causes are distinguished
`interrupt_handler()` MUST 根据屏蔽最高位后的中断原因区分 `IRQ_S_TIMER`、`IRQ_S_SOFT`、`IRQ_S_EXT` 和未知中断。软件中断 MUST 输出 `Supervisor software interrupt`，外部中断 MUST 输出 `Supervisor external interrupt`，未知中断 MUST 输出 `Unknown interrupt: cause <n>` 并调用 `print_trapframe(tf)`。

#### Scenario: Software interrupt
- **WHEN** `interrupt_handler()` 收到带中断最高位的 `IRQ_S_SOFT`
- **THEN** 输出 `Supervisor software interrupt`

#### Scenario: Unknown interrupt
- **WHEN** `interrupt_handler()` 收到原因号为 15 的未知中断
- **THEN** 输出 `Unknown interrupt: cause 15`，并输出该 trapframe

### Requirement: Interrupt handling stays within its module boundary
本 change MUST 只实现 `interrupt_handler()`；`exception_handler()` MUST 保持未实现，`trapentry.S` MUST 保持不变，lab2 代码 MUST NOT 进入提交。

#### Scenario: Review the code diff
- **WHEN** 检查本 change 的代码差异
- **THEN** 只包含 `lab3/kern/trap/trap.c` 中的中断处理实现、必要的声明/宏/头文件，以及 OpenSpec 产物
