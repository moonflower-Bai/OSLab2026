# Spec Delta

## Purpose

规定 Lab3 异常处理单元测试的通过条件与失败输出，使宿主机可以直接判断异常分派、`epc` 推进和 trapframe 不变式，并保留尚未执行的 QEMU 保存/恢复与委托测试。

## ADDED Requirements

### Requirement: Host test entry

`bash lab3/test/exception.sh` MUST 先跑完宿主机测试，再编译 QEMU 层。找不到 `riscv64-unknown-elf-gcc` 时 MUST 只打印一行提示并跳过 QEMU 层，且 MUST NOT 因此使脚本失败。工具链可用时 MUST 用 `ld --wrap=trap --wrap=kern_init` 链接 QEMU 层目标文件，并输出「QEMU层仅链接检查」。脚本注释 MUST 写明在完整内核上的启动命令。设置 `EXCEPTION_RUN_QEMU=1` 且 `EXCEPTION_QEMU_IMG` 指向镜像时 MUST 启动该镜像；成功条件 MUST 是「通过」行数等于该层全部检查项，并且没有「错误」行。宿主机检查本身 MUST NOT 依赖 QEMU 进程。

任一宿主机检查失败、无法得到被测异常处理或 QEMU 层无法编译、链接时，进程 MUST 以非零状态退出。每一项结果 MUST 标明「通过」或「错误」。失败项 MUST 输出当前错误值，且 MUST NOT 输出期待值。测试 MUST NOT 使用恒真断言。

完整内核上的启动命令是：

`qemu-system-riscv64 -machine virt -nographic -bios default -device loader,file=<ucore.img>,addr=0x80200000`

镜像用 `ld --wrap=trap --wrap=kern_init -T lab3/tools/kernel.ld` 链接。

#### Scenario: Command checks the host and links the QEMU objects

- **WHEN** 执行 `bash lab3/test/exception.sh` 且 RISC-V 工具链可用
- **THEN** 宿主机逐项输出「通过」或「错误」，并且 QEMU 层目标文件以 `--wrap=trap` 完成链接

#### Scenario: Failure reports the observed value

- **WHEN** 某一项检查失败
- **THEN** 该项输出包含当前错误值，且不包含期待值

### Requirement: Breakpoint handling

断点异常 MUST 输出包含 `Breakpoint at` 的文本，其中的地址 MUST 等于进入处理时的 `epc`。处理返回后 `epc` MUST 按该指令长度前进，且 MUST NOT 调用 `panic`。`epc` MUST 指向测试提供的本地指令编码，MUST NOT 为 0。

#### Scenario: Four-byte ebreak

- **WHEN** cause 为断点，且 `epc` 指向编码 `0x00100073`
- **THEN** 输出包含 `Breakpoint at` 且不含 `Illegal instruction at`，地址等于进入时的 `epc`，返回后 `epc` 增加 4，并且 `sstatus.SPP` 为 0 或 1 时都不 panic

#### Scenario: Compressed ebreak

- **WHEN** cause 为断点，且 `epc` 指向编码 `0x9002`
- **THEN** 输出包含 `Breakpoint at` 且不含 `Illegal instruction at`，地址等于进入时的 `epc`，返回后 `epc` 增加 2，并且 `sstatus.SPP` 为 0 或 1 时都不 panic

### Requirement: Illegal instruction handling

非法指令异常 MUST 输出包含 `Illegal instruction at` 的文本，其中的地址 MUST 等于进入处理时的 `epc`。处理返回后 `epc` MUST 按该指令长度前进，且 MUST NOT 调用 `panic`。`epc` MUST 指向测试提供的本地指令编码，MUST NOT 为 0。

#### Scenario: Four-byte illegal instruction

- **WHEN** cause 为非法指令，且 `epc` 指向编码 `0x0000007b`（低两位为 `0b11`，低 7 位不是全 1，`inst[4:2]` 不是 `111`）
- **THEN** 输出包含 `Illegal instruction at` 且不含 `Breakpoint at`，地址等于进入时的 `epc`，返回后 `epc` 增加 4，并且 `sstatus.SPP` 为 0 或 1 时都不 panic

#### Scenario: Two-byte illegal instruction

- **WHEN** cause 为非法指令，且 `epc` 指向编码 `0x0000`
- **THEN** 输出包含 `Illegal instruction at` 且不含 `Breakpoint at`，地址等于进入时的 `epc`，返回后 `epc` 增加 2，并且 `sstatus.SPP` 为 0 或 1 时都不 panic

#### Scenario: Two-byte illegal instruction with low bits 01

- **WHEN** cause 为非法指令，且 `epc` 指向编码 `0x6101`
- **THEN** 输出包含 `Illegal instruction at` 且不含 `Breakpoint at`，地址等于进入时的 `epc`，返回后 `epc` 增加 2，并且 `sstatus.SPP` 为 0 或 1 时都不 panic

### Requirement: Instruction-length epc advance

`epc` 前进量 MUST 由 `epc` 指向的第一条 16 位半字决定：低两位为 `0b11` 时前进 4，否则前进 2。测试 MUST 分别使用 `ebreak`（`0x00100073`）、`c.ebreak`（`0x9002`）、32 位长度编码的非法指令 `0x0000007b`、16 位非法指令 `0x0000`，以及低两位为 `0b01` 的保留编码 `0x6101`。`0x0000007f` 不得当作 4 字节指令，`0x0001` 不得当作非法指令。

#### Scenario: Length follows the first halfword

- **WHEN** 被测处理根据指令编码更新断点或非法指令的 `epc`
- **THEN** `0x00100073` 与 `0x0000007b` 使 `epc` 增加 4，`0x9002`、`0x0000` 与 `0x6101` 使 `epc` 增加 2

### Requirement: Selective epc read

被测处理 MUST 只在断点与非法指令两类已识别异常上读取 `epc` 指向的内存。未知 cause MUST NOT 读取该地址。

#### Scenario: Unknown cause does not dereference epc

- **WHEN** cause 不是断点或非法指令，且 `epc` 指向不可读内存或为 0
- **THEN** 处理不因读取 `epc` 而失败

### Requirement: Unknown exception from user mode

未知 cause 且来自用户态（`sstatus.SPP` 为 0）时，处理 MUST 调用 `print_trapframe` 后正常返回，MUST NOT 调用 `panic`，且 MUST NOT 推进 `epc`。cause 为 20 且 trapframe 其余字段为 0 属于该情形。

#### Scenario: Zeroed frame with cause 20

- **WHEN** cause 为 20 且 trapframe 的其余字段为 0
- **THEN** 输出包含 `print_trapframe` 产生的 cause 行，处理正常返回，`epc` 仍为 0，并且没有 panic

### Requirement: Unknown exception from kernel mode

未知 cause 且来自内核态（`sstatus.SPP` 为 1）时，处理 MUST 先调用 `print_trapframe`，然后 MUST 调用 `panic`，且 MUST NOT 推进 `epc`。

#### Scenario: Kernel unknown cause panics after printing

- **WHEN** cause 不是断点或非法指令，且 `sstatus.SPP` 为 1
- **THEN** 输出包含 cause 行，随后发生 panic，并且 `epc` 保持进入时的值

### Requirement: Unknown cause line

未知异常路径 MUST 调用 `print_trapframe`。输出 MUST 包含 `cause    0x%08x` 这一格式所产生的 cause 行，且该数值 MUST 等于 trapframe 的 cause。

#### Scenario: Printed cause matches the frame

- **WHEN** 处理一条未知异常
- **THEN** 输出中的 cause 行数值等于进入时 trapframe 的 cause

### Requirement: Cause dispatch for both privilege modes

每个被测 cause 在 `sstatus.SPP` 为 0 和为 1 时都 MUST 各测一次。这里的「返回」MUST 表示不调用 `panic`。输出 MUST NOT 包含其他 cause 的专属字符串：断点的专属字符串是 `Breakpoint at`，非法指令的专属字符串是 `Illegal instruction at`。

cause 为 2 时 MUST 按非法指令处理，`epc` 按指令长度推进，两种 SPP 都返回。cause 为 3 时 MUST 按断点处理，`epc` 按指令长度推进，两种 SPP 都返回。cause 为 0、1、4、5、6、7、8、9、11、12、13、15 时 MUST 调用 `print_trapframe`，`epc` 不变，SPP 为 0 时返回，SPP 为 1 时 panic。cause 为 10、14 以及 16 和更大的非中断值（至少包括 16、20、31、63）MUST 与上述未知异常相同。

#### Scenario: Illegal instruction and breakpoint return in both modes

- **WHEN** cause 为 2 或 3，且 `sstatus.SPP` 分别为 0 和 1
- **THEN** 走对应分支，输出不含另一类专属字符串，并且不 panic

#### Scenario: Other defined causes

- **WHEN** cause 为 0、1、4、5、6、7、8、9、11、12、13 或 15
- **THEN** 输出包含 cause 行且不含 `Breakpoint at` 与 `Illegal instruction at`，`epc` 不变；SPP 为 0 时返回，SPP 为 1 时 panic

#### Scenario: Reserved causes

- **WHEN** cause 为 10、14、16、20、31 或 63
- **THEN** 按未知异常处理：调用 `print_trapframe`，`epc` 不变，输出不含断点或非法指令专属字符串；SPP 为 0 时返回，SPP 为 1 时 panic

### Requirement: Trapframe fields stay unchanged

除允许推进的 `epc` 外，trapframe 的其余字段在处理前后 MUST 保持不变，包括通用寄存器和 status。未知异常的 `epc` 也 MUST 保持不变。

#### Scenario: Registers and status are preserved

- **WHEN** 任意被测异常处理返回或 panic
- **THEN** 除断点或非法指令被允许修改的 `epc` 外，trapframe 各字段与进入时相同

### Requirement: Incorrect handlers are rejected

测试 MUST 至少对下列每一种错误实现给出一项失败：`epc` 不推进、一律加 4、一律加 2、断点与非法指令处理互换、未知异常未调用 `print_trapframe`、未知异常无条件 panic、未知异常无条件返回、`trap_in_kernel` 判断取反、修改了 trapframe 的其他字段。

#### Scenario: Each incorrect variant fails

- **WHEN** 被测实现属于上述任一种错误行为
- **THEN** 至少一项宿主机检查输出「错误」

### Requirement: Interrupt path is stubbed

宿主机测试 MUST 桩住 `interrupt_handler`、`panic` 和 `cprintf`，并捕获 `cprintf` 的输出。测试 MUST NOT 补全 `interrupt_handler`。异常用例 MUST NOT 依赖中断处理逻辑。

#### Scenario: Exception cases do not enter the interrupt stub

- **WHEN** 运行 cause 非负的异常用例
- **THEN** 中断桩不被调用，输出来自 `cprintf` 桩

### Requirement: QEMU image is linked and can be run

`bash lab3/test/exception.sh` MUST 先完成宿主机测试，再编译 QEMU 层源码，并以 `ld --wrap=trap --wrap=kern_init` 链接这些目标文件。找不到 `riscv64-unknown-elf-gcc` 时 MUST 跳过该层且不因此失败。链接成功时 MUST 输出「QEMU层仅链接检查」。进入 `trap()` 时，trapframe 中除 `sp` 外的通用寄存器 MUST 等于探针写入的哨兵，`sp` 槽 MUST 等于进入前的 `sp`，`cause` MUST 为 3，`status` 的 SPP 位 MUST 为 1。保存的 `x0` MUST 为 0。脚本注释 MUST 写明完整内核镜像的链接和启动方式；设置 `EXCEPTION_RUN_QEMU=1` 且 `EXCEPTION_QEMU_IMG` 指向镜像时 MUST 启动该镜像，并且只有「通过」行数等于全部检查项且没有「错误」行时才算成功。

保存与恢复：trapframe MUST 为 36 个指针槽，通用寄存器位于 0 到 31，status、`epc`、badvaddr、cause 依次位于 32 到 35。进入 `trap()` 时收到的指针 MUST 等于当时的 `sp`，且保存的 `x0` MUST 为 0。恢复后 `sp` MUST 等于进入前的 `sp`，且除 `epc` 外 `x1` 到 `x31` MUST 保持不变。进入 `trap()` 时 `sscratch` MUST 为 0，返回前 MUST 恢复为 0，使嵌套异常被识别为来自内核。

异常委托：测试 MUST NOT 在 S 态读取 `medeleg`。在 S 态执行 `ebreak` 时，MUST 直接进入 S 模式，表现为 `scause` 为 3、`sstatus.SPP` 为 1，并且返回后继续执行下一条指令。

#### Scenario: Script links the QEMU objects

- **WHEN** 执行 `bash lab3/test/exception.sh` 且 RISC-V 工具链可用
- **THEN** QEMU 层目标文件完成编译，并用 `--wrap=trap` 完成链接

#### Scenario: Saved x0 is zero

- **WHEN** 进入 `trap()` 的 trapframe 被观察
- **THEN** 槽位 0 的保存值是 0

#### Scenario: S-mode ebreak is delegated by behavior

- **WHEN** 在链接好的镜像中于 S 态执行 `ebreak`
- **THEN** S 态陷阱观察到 `scause` 为 3 且 `sstatus.SPP` 为 1，返回后从下一条指令继续执行
