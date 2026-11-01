# User Prompts

change: lab3-exception-unit-tests
reviewed_for_relevance: true
reviewed_for_sensitive_data: true

## 2026-10-27T22:26:00+08:00

[prompt]
请你根据当前项目内容，实现对RISC-V 上操作系统的异常处理的单元测试。测试主要包括模式切换，恢复状态时的指针地址是否正确，以及各个异常处理逻辑是否正确。
[修改文件]
- 创建lab3/test/exception文件夹以及lab3/test/exception.sh文件
- 在exception文件夹内设计单元测试，最终可以通过exception.sh直接执行所有测试，并且输出测试结果是通过还是错误。如果错误，不输出期待值，但是输出当前错误值。
- 不允许修改lab3以外的所有文件
- 中断不是本次任务，不需要补全interrupt_handler，测试中用桩代替
[可读范围]
- 不允许读取lab3/tools文件夹
[不变量]
- 除本次实现的单元测试外均为不变量
[验收规则]
- 可以使用bash lab3/test/exception.sh指令直接测试当前lab3异常实现的正确性
- 能够正确检查出异常实现的正确性
[测试要求]
- 测试分两层。宿主机层不依赖QEMU，桩住interrupt_handler、panic、cprintf并捕获输出；QEMU层需要完整内核，暂时只编写，待依赖的分支合入后再运行。
- 使用子代理对所有已经实现的单元测试进行对抗测试，检查是否有错误情况没有被成功检查。子代理不允许读取测试代码，只负责生成若干个故意写错的exception_handler变体，变体放在临时目录，不进入仓库。每个变体运行测试时必须至少有一项失败，否则视为测试漏洞。
- 不允许使用assert(true)或类似逻辑等价代码，包括简化后为true的条件以及不写类似代码。具体实现以测试框架使用的assert或类似函数为准。

[宿主机层]
- 断点：输出 Breakpoint at，并断言输出中的地址等于传入的epc；epc按指令长度推进后返回。
- 非法指令：输出 Illegal instruction at，并断言输出中的地址等于传入的epc；epc按指令长度推进后返回。
- epc推进量按指令长度：第一个16位半字低两位为0b11推进4，否则推进2（编译器默认rv64gc，含压缩指令）。分别构造ebreak(0x00100073)、c.ebreak(0x9002)以及非法指令的4字节与2字节编码；tf.epc指向测试内的本地数组，不得解引用0地址。
- 只在已识别的CAUSE_BREAKPOINT与CAUSE_ILLEGAL_INSTRUCTION分支读取epc指向的内存，未知cause分支不得读取。
- 未知cause且trap_in_kernel(tf)为假（SPP=0，来自用户态）：调用print_trapframe(tf)后正常返回，不panic，不推进epc。cause=20且tf清零属于此情形。
- 未知cause且trap_in_kernel(tf)为真（SPP=1，来自内核态）：调用print_trapframe(tf)后panic。
- 未知异常分支必须调用print_trapframe；断言输出中出现 "cause    0x%08x" 且值等于tf.cause。
- cause取0..15全部值：2、3走各自分支，其余按未知异常处理。
- 除epc外，tf的其余字段处理前后必须不变（防止误改gpr、status）。

[QEMU层]
- 保存与恢复：以偏移断言trapframe为36个槽，gpr在0..31，status、epc、badvaddr、cause在32..35；断言trap()收到的tf等于进入时的sp。
- 恢复后sp与进入前相等；除epc外x1..x31全部不变。
- 进入trap时sscratch为0，返回前恢复，使嵌套异常能被识别为来自内核。
- 异常委托：medeleg是M态寄存器，S态不得读取。以行为验证委托是否生效：在S态触发ebreak，断言直接陷入S模式，即scause=3、sstatus.SPP=1，且返回后继续执行下一条指令。

[反向用例]
测试必须能对以下变体报错，每个至少一个：
- epc不推进
- 一律+4
- 一律+2
- 断点与非法指令的处理互换
- 未知异常漏调print_trapframe
- 未知异常无条件panic
- 未知异常无条件返回
- trap_in_kernel判断取反
- 修改了tf的其他字段

## 2026-10-27T23:15:00+08:00

请对 lab3/test/exception.sh 及 lab3/test/exception/ 做以下修改。范围仍然只限这些文件，不要读 lab3/tools，不要修改 lab3 以外的文件。

  1. 脚本顺序（必须）
  - 先运行宿主机测试，再编译 QEMU 层。
  - 找不到 riscv64-unknown-elf-gcc 时，QEMU 层只打印一行提示并跳过，不要让脚本失败，宿主机层的结果必须能单独得到。
  - 把"通过 QEMU链接"改成能说明它只是链接检查的文字，例如"QEMU层仅链接检查"，不要让它看起来像行为验证。

  2. 指令长度用例（必须）
  - 再加一个低两位为 01 的 16 位编码，用 cause=2 测试 epc 推进 2。可以用 0x6101，它是 c.addi16sp 且立即数为 0 的保留编码；请对照 RISC-V 手册确认。
  - 目的：让"只看指令第 0 位判断长度"的错误实现被抓出来。

  3. QEMU 层 gp、tp 处理
  - 在 __wrap_trap 里，调用 __real_trap 之前把 gp、tp 恢复成原始值，调用返回之后再放回哨兵值。
- __wrap_trap 返回后由 RESTORE_ALL 从 tf 恢复寄存器，所以这样不会影响恢复测试。

4. QEMU 层 tf 槽位检查
- 在 __wrap_trap 里，进入时检查以下内容：
  - tf->gpr.xN 等于探针写入的哨兵值（sp 除外）。
  - tf->gpr.sp 等于 qemu_sp_before。
  - tf->cause 等于 3，且 tf->status 的 SPP 位为 1。
- 目的：避免 SAVE_ALL 与 RESTORE_ALL 使用同一套错误偏移时，来回一圈仍然通过。

5. QEMU 运行结果判定
- run_qemu 里不要再用"至少一行通过"判断成功，改成"通过"行数等于预期总数，并且没有任何"错误 "行。

6. 低优先级
- 探针里的 ebreak 用 .option push、.option norvc、.option pop 固定为 4 字节，避免汇编结果依赖工具链默认 arch。
- 以上每项修改后，原有的反向用例（epc 不推进、一律 +4、一律 +2、断点与非法指令互换、未知异常漏调 print_trapframe、无条件 panic、无条件返回、trap_in_kernel 取反、修改其他字段）必须仍然被抓出来。


## 2026-10-27T23:05:00+08:00

- 逐个 cause 的期望，SPP=0 与 SPP=1 各测一遍；"返回"指不 panic：

  | cause | 名称 | 输出 | epc | SPP=0 | SPP=1 |
  |---|---|---|---|---|---|
  | 2 | 非法指令 | Illegal instruction at | 按指令长度推进 | 返回 | 返回 |
  | 3 | 断点 | Breakpoint at | 按指令长度推进 | 返回 | 返回 |
  | 0,1,4,5,6,7,8,9,11,12,13,15 | 其余已定义异常 | print_trapframe | 不变 | 返回 | panic |
  | 10,14,16 及以上 | 保留或未定义 | print_trapframe | 不变 | 返回 | panic |

- 每个 cause 都要断言：输出中不含其他 cause 的专属字符串（例如断点不得输出 Illegal instruction at）。
需要你对照手册核对的问题

1. 4 字节非法指令的编码 0x0000007f 可能不是 32 位指令。
- RISC-V 的指令长度编码里，低 7 位全为 1 属于保留的 ≥192 位长编码。官方测试用的 0x7b 才是合法的 32 位长度编码。
- 在你们简化的"低两位为 11 就推进 4"的规则下它能过，但它在语义上不代表一条 4 字节指令。建议改回 0x0000007b，或者换一个低 7 位是标准 32 位长度编码的值。

2. 2 字节"非法指令"用的 0x0001 其实是合法的 c.nop。
- 16 位全 0（0x0000）才是规范定义的非法指令。
- 在宿主机上，这只是字节，不影响断言。但标签叫"非法-2字节"，将来有人把它放到 QEMU 里真的执行就不会触发异常。

测试覆盖的缺口

- 缺少"不输出另一类标记"的未知分支检查。 run_unknown 只检查 cause    0x... 那一行，没有断言未知异常的输出里不含 Breakpoint at 和 Illegal instruction at。
- exception.sh 不编译 QEMU 层。 355 行代码完全没经过编译，语法或链接错误发现不了。文件里写了需要 -Wl,--wrap=trap，但仓库里没有对应的构建或运行脚本，所以现在这层是"只写不跑"，也没有说明怎么跑。
- x0 的保存没被验证。 QEMU 层的 report_reg 从 x1 开始检查。

实现质量方面

- host_test.c 里自己手写了 join、find_str、parse_hex、parse_number，约 100 行，用 string.h 和 strtoull 也能完成，是可以删减的部分。
- exception.sh 依赖 llvm-objcopy 把 interrupt_handler 弱化成桩，macOS 默认没有这个工具，换台机器会直接报"错误 工具"。
- 编译选项里放宽了 -Wno-implicit-function-declaration 等，会掩盖 trap.c 里真正的隐式声明错误。
- host_include/atomic.h 通过 -I 的顺序遮蔽了真实的 libs/atomic.h。

