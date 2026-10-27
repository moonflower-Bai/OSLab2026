# User Prompts

change: lab3-exception-handler
reviewed_for_relevance: true
reviewed_for_sensitive_data: true

## 2026-10-27T23:12:00+08:00

[Prompt]
请你根据当前项目内容，完成一个RISCV上操作系统的异常处理。异常处理函数签名是void exception_handler(struct trapframe *tf);
中断不是本次任务，不要实现interrupt_handler。
[修改文件]
- 目标修改文件仅限lab3/kern/trap/trap.c以及lab3/kern/trap/trapentry.S两个文件
- 不允许修改lab3以外的所有文件
- 不需要补全interrupt_handler
- trapentry.S 只补全 SAVE/RESTORE 之外的缺口，不做中断和异常分流（该分流由trap.c中的trap_dispatch负责），不留 TODO。
[行为要求]
- 断点与非法指令的处理、未知异常的分支规则，以「测试」一节的宿主机层为准
- 未知异常分支是否panic由trap_in_kernel(tf)决定
[不变量]
- 除异常处理外均为不变量
[验收规则]
- 满足lab3/test/exception目录下的所有单元测试结果。且在你实现期间不允许读取该测试详细信息，只允许执行bash lab3/test/exception.sh指令来检查结果是否正确。
