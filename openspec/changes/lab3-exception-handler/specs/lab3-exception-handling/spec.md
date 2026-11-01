# Spec Delta

## Purpose

规定 Lab3 异常路径在进入已有陷阱分派之后的可观察结果：断点与非法指令如何报告并推进 `epc`，以及未知异常何时打印现场、何时 panic。

## ADDED Requirements

### Requirement: Exception entry reaches the dispatcher

保存现场之后，陷阱入口 MUST 把当前栈上的 trapframe 交给已有的陷阱分派，并且 MUST NOT 在汇编入口区分中断与异常。调用返回后栈指针 MUST 仍指向同一 trapframe。中断仍由已有分派交给中断处理，本能力 MUST NOT 定义中断处理的行为。

#### Scenario: Saved frame is dispatched

- **WHEN** 陷阱入口已经保存现场且栈指针指向该 trapframe
- **THEN** 分派收到的指针等于当时的栈指针，中断与异常的区分发生在已有的 C 语言分派中

### Requirement: Breakpoint handling

断点异常 MUST 输出包含 `Breakpoint at` 的文本，其中的地址 MUST 等于进入处理时的 `epc`。处理返回后 `epc` MUST 按该指令长度前进，且 MUST NOT 调用 `panic`。`sstatus.SPP` 为 0 或 1 时都 MUST 返回。输出 MUST NOT 包含 `Illegal instruction at`。

#### Scenario: Four-byte ebreak

- **WHEN** cause 为 3，且 `epc` 指向编码 `0x00100073`
- **THEN** 输出包含 `Breakpoint at` 且不含 `Illegal instruction at`，地址等于进入时的 `epc`，返回后 `epc` 增加 4，并且两种 `sstatus.SPP` 都不 panic

#### Scenario: Compressed ebreak

- **WHEN** cause 为 3，且 `epc` 指向编码 `0x9002`
- **THEN** 输出包含 `Breakpoint at` 且不含 `Illegal instruction at`，地址等于进入时的 `epc`，返回后 `epc` 增加 2，并且两种 `sstatus.SPP` 都不 panic

### Requirement: Illegal instruction handling

非法指令异常 MUST 输出包含 `Illegal instruction at` 的文本，其中的地址 MUST 等于进入处理时的 `epc`。处理返回后 `epc` MUST 按该指令长度前进，且 MUST NOT 调用 `panic`。`sstatus.SPP` 为 0 或 1 时都 MUST 返回。输出 MUST NOT 包含 `Breakpoint at`。地址 MUST 打印全部比特，使宿主机上高于 4GiB 的 `epc` 仍与进入时的数值相等。

#### Scenario: Four-byte illegal instruction

- **WHEN** cause 为 2，且 `epc` 指向编码 `0x0000007b`
- **THEN** 输出包含 `Illegal instruction at` 且不含 `Breakpoint at`，地址等于进入时的 `epc`，返回后 `epc` 增加 4，并且两种 `sstatus.SPP` 都不 panic

#### Scenario: Two-byte illegal instruction

- **WHEN** cause 为 2，且 `epc` 指向编码 `0x0000`
- **THEN** 输出包含 `Illegal instruction at` 且不含 `Breakpoint at`，地址等于进入时的 `epc`，返回后 `epc` 增加 2，并且两种 `sstatus.SPP` 都不 panic

### Requirement: Instruction-length epc advance

断点与非法指令的 `epc` 前进量 MUST 由 `epc` 指向的第一条 16 位半字决定：低两位为 `0b11` 时前进 4，否则前进 2。只有这两类已识别异常 MUST 读取该地址。未知 cause MUST NOT 读取 `epc` 指向的内存。

#### Scenario: Length follows the first halfword

- **WHEN** 处理断点或非法指令并更新 `epc`
- **THEN** `0x00100073` 与 `0x0000007b` 使 `epc` 增加 4，`0x9002` 与 `0x0000` 使 `epc` 增加 2

#### Scenario: Unknown cause does not dereference epc

- **WHEN** cause 不是 2 或 3，且 `epc` 指向不可读内存或为 0
- **THEN** 处理不因读取 `epc` 而失败

### Requirement: Unknown exception

cause 不是 2 或 3 时，处理 MUST 调用 `print_trapframe`，输出 MUST 包含 `cause    0x%08x` 这一格式所产生的 cause 行，且该数值 MUST 等于 trapframe 的 cause。`epc` MUST 保持不变。是否 panic MUST 由 `trap_in_kernel(tf)` 决定：其为真时 MUST 在打印之后 panic，其为假时 MUST NOT panic 并正常返回。输出 MUST NOT 包含 `Breakpoint at` 或 `Illegal instruction at`。

cause 为 0、1、4、5、6、7、8、9、10、11、12、13、14、15、16、20、31、63 都属于未知异常。

#### Scenario: User unknown cause returns

- **WHEN** cause 为 20，trapframe 其余字段为 0，因而 `trap_in_kernel(tf)` 为假
- **THEN** 输出包含 cause 行，处理正常返回，`epc` 仍为 0，并且没有 panic

#### Scenario: Kernel unknown cause panics after printing

- **WHEN** cause 不是 2 或 3，且 `trap_in_kernel(tf)` 为真
- **THEN** 输出包含 cause 行，随后发生 panic，并且 `epc` 保持进入时的值

### Requirement: Trapframe fields stay unchanged

除断点或非法指令被允许推进的 `epc` 外，trapframe 的其余字段在处理前后 MUST 保持不变，包括通用寄存器和 status。未知异常的 `epc` 也 MUST 保持不变。

#### Scenario: Registers and status are preserved

- **WHEN** 任意被测异常处理返回或 panic
- **THEN** 除断点或非法指令被允许修改的 `epc` 外，trapframe 各字段与进入时相同
