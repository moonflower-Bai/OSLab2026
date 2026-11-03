# Lab3 提示词整理

本文按报告中的模块整理用于 AI 协作的提示词。功能实现、接口约束和验收标准取自本次实验的记录；中断部分把分步实施要求汇总成一份等价规格，原始分步记录见 `openspec/changes/implement-lab3-interrupt-handler/prompt.md`。当前报告整理与补充验证使用 Kimi Code CLI，它不代表先前各模块的原始生成工具。

## 一、Lab2 依赖回填

````markdown
[任务]
将 Lab2 已完成并验证的物理内存初始化与 Best-Fit 实现迁移到 Lab3，
保留 Lab3 原有的页表初始化和中断保护包装。

[约束]
1. 只迁移 page_init()、Best-Fit 实现和管理器选择。
2. 保留 Lab3 的 alloc_pages/free_pages/nr_free_pages 中断保护。
3. 不引入 Buddy，不修改官方检查函数和评分脚本。
4. 使用 DTB 获取内存范围，保留固件、内核和页描述符占用的页。

[验收]
先编译 PMM 对象，再完整构建。内核启动应输出物理内存范围、
空闲页数量以及 check_alloc_page() succeeded!。
````

## 二、Trap 上下文恢复与 Trap 入口（何闻硕）

````markdown
[任务]
实现 kern/trap/trapentry.S 中的 RESTORE_ALL 和 __alltraps。

[已有条件]
SAVE_ALL 已在当前内核栈上建立 struct trapframe，执行后 sp 指向帧首。
Trap frame 有 36 个 REGBYTES 槽位：0～31 对应 x0～x31，
32～35 对应 sstatus、sepc、badvaddr、scause。
原始 sp 经 sscratch 和 s0 中转后存放在槽位 2。
C 处理接口为 void trap(struct trapframe *tf)。

[实现目标]
1. RESTORE_ALL 按 SAVE_ALL 的偏移恢复 Trap 前的上下文。
2. __alltraps 将当前 Trap frame 地址传给 trap()。
3. trap() 返回后进入 __trapret，恢复上下文并通过 sret 返回。

[约束]
1. 不修改 SAVE_ALL、struct trapframe 或 trap() 的签名。
2. 恢复 sp 前必须完成所有以当前帧为基址的读取。
3. 使用通用寄存器中转 sstatus、sepc、badvaddr，再恢复这些寄存器。
4. 通过 a0 传递 Trap frame 地址，遵守 RISC-V 调用约定。
5. 不在汇编入口处理中断、异常或修改 epc。
6. 若工具链不接受旧 CSR 名称，仅作 sbadaddr 到 stval 的兼容替换。

[验收]
1. trapentry.S 可独立编译。
2. Trap 前后通用寄存器与原始 sp 保持一致。
3. trap() 收到的 tf 等于 SAVE_ALL 完成后的 sp。
4. 与异常处理集成后，能从断点异常返回下一条指令，不重复陷入。
5. 运行完整构建、QEMU 与评分测试。
````

## 三、异常处理与测试（白景仁）

### 3.1 测试提示词

````markdown
[任务]
为 RISC-V 操作系统的异常处理实现单元测试，覆盖模式切换、上下文
保存恢复和异常处理逻辑。先完成测试，再依据测试结果实现功能。

[文件与范围]
1. 创建 lab3/test/exception/ 和 lab3/test/exception.sh。
2. 不修改 lab3 以外的文件；中断不是本次任务，用桩替代。
3. 不读取 lab3/tools；除本次测试外均保持为不变量。
4. 测试失败时输出用例名和实测值，不输出期待值。
5. 不使用 assert(true) 或逻辑等价的恒真断言。

[宿主机层]
1. 不依赖 QEMU，编译真实 trap.c，桩住 interrupt_handler、panic、cprintf。
2. 断点、非法指令分别输出 Breakpoint at / Illegal instruction at，
   地址必须等于传入的完整 64 位 epc。
3. 第一条 16 位半字低两位为 0b11 时 epc 加 4，否则加 2。
4. 覆盖 ebreak(0x00100073)、c.ebreak(0x9002) 和非法的 4/2 字节编码；
   epc 指向真实测试数组，不解引用空地址。
5. 仅在已识别的断点和非法指令分支读取 epc 指向的内存。
6. 未知异常：先 print_trapframe；来自用户态正常返回，来自内核态 panic；
   两种情况均不推进 epc。cause=20 且帧清零也应安全处理。
7. 覆盖 cause 0～15，检查除 epc 外的帧字段不变。

[QEMU 层]
1. 静态检查 trapframe 为 36 个槽位及全部字段偏移。
2. 断言 trap() 收到的 tf 等于进入时 sp。
3. 返回后 sp 与进入前一致，除 epc 外 x1～x31 不变。
4. 进入 trap 时 sscratch 为 0，返回前恢复。
5. 不在 S 态读取 medeleg；通过 ebreak 直接进入 S 态、scause=3、
   sstatus.SPP=1 且能返回下一条指令来验证委托。
6. 在依赖分支合入前可先只做交叉编译和链接检查。

[对抗验收]
bash lab3/test/exception.sh 可直接运行。对临时目录中的错误变体逐一测试：
不推进 epc、一律加 4、一律加 2、交换两类异常、漏打印未知异常、
未知异常无条件 panic/返回、模式判断取反、修改其他帧字段。
每个变体至少应失败一项，变体不进入仓库。
````

### 3.2 实现提示词

````markdown
[任务]
实现 void exception_handler(struct trapframe *tf)，完成 RISC-V 异常处理。

[文件与范围]
1. 只修改 lab3/kern/trap/trap.c 和 trapentry.S。
2. 不实现 interrupt_handler；不在 trapentry.S 做异常/中断分流。
3. trapentry.S 只补 SAVE_ALL/RESTORE_ALL 之外的调用缺口。
4. 除异常处理外均保持为不变量。

[行为]
1. cause=CAUSE_ILLEGAL_INSTRUCTION：
   输出 Illegal instruction at 0x%016llx，按指令长度推进 epc。
2. cause=CAUSE_BREAKPOINT：
   输出 Breakpoint at 0x%016llx，按指令长度推进 epc。
3. 第一条 16 位半字低两位为 0b11 时加 4，否则加 2。
4. 其余 cause 先 print_trapframe(tf)，再由 trap_in_kernel(tf)
   决定是否 panic；不读取未知异常的 epc，不推进它。
5. 地址输出必须保留完整 64 位。

[验收]
运行 bash lab3/test/exception.sh。实现期间不读取测试细节，
只根据执行结果修正，直到宿主机检查和 QEMU 链接检查通过。
````

## 四、中断处理（陈彦兴）

````markdown
[任务]
只实现功能模块二中的 interrupt_handler(struct trapframe *tf)，
不实现上下文恢复，也不实现 exception_handler。

[接口与常量]
1. tf->cause 的最高位标识中断；IRQ_S_* 原因号不含该位。
2. 使用 clock_set_next_event()、ticks、TICK_NUM、print_trapframe(tf)。
3. 使用 libs/sbi.h 中的关机接口；SHUTDOWN_AFTER 取 10。

[行为]
1. 先清除 cause 的最高位，再按原因号 switch。
2. IRQ_S_TIMER：先设置下一次时钟事件，再 ticks++；
   每 TICK_NUM 次调用 print_ticks()；累计输出达到 SHUTDOWN_AFTER
   后调用 sbi_shutdown()。
3. IRQ_S_SOFT：输出 Supervisor software interrupt。
4. IRQ_S_EXT：输出 Supervisor external interrupt。
5. 未知中断：输出 Unknown interrupt: cause <n>，并打印完整 trapframe。
6. 保持 trap_dispatch()、trap() 和函数签名不变。

[分步与验收]
1. 先识别中断类型并覆盖软件、外部和未知中断。
2. 再实现时钟重设、计数和周期输出。
3. 最后加入统计阈值关机。
4. 每一步独立编译 trap.o，再运行完整内核。
5. 原始 Test 3 应看到 100 ticks 和 ticks=200；长跑在第十次 100 ticks
   输出后自动关机。依赖分支未合入时，只在仓库外临时副本补齐验证。
````

## 五、报告整理与补充验证

````markdown
请你根据实验报告模板以及其他分支里的实验报告，帮我完成本次的实验报告。

以 Lab2 报告的结构和 Lab1/Lab2 已确认的成员、工具信息为参考，
结合 Lab3 实际代码、指导书和实验记录，补齐功能实现、提示词、
迭代问题、测试截图和总结。报告中的测试结果必须来自真实运行，
区分原始命令失败、临时启动参数调整和最终通过的测试。
````
