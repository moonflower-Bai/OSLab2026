# Spec Delta

## Purpose

规定 Lab1 环境、构建、启动和 GDB 验证的可观察结果，使小组成员能够从仓库复现内核启动并解释证据；明确缺少工具、启动失败与正常循环的区别，避免把未执行的检查或入口观察等同于完整实验验收。

## ADDED Requirements

### Requirement: Report environment prerequisites
环境检查入口 MUST 报告构建、运行和调试工具的实际可用状态及架构支持；必要条件缺失时 MUST 返回非零状态并列出缺项，MUST NOT 自动安装软件或修改用户全局配置。

#### Scenario: Required cross compiler is absent
- **WHEN** 用户运行环境检查且 RISC-V 编译器不可用
- **THEN** 检查返回非零状态并明确报告缺少编译器

#### Scenario: A compatible debugger is available
- **WHEN** 用户拥有能够处理 RV64 的 GDB
- **THEN** 检查报告实际使用的调试器及 RV64 支持结果

### Requirement: Verify bounded kernel boot
启动验证 MUST 在有限时长内结束，并报告构建状态、运行命令和内核输出。只有构建成功且观察到指定内核横幅时才 MUST 返回成功；仅有固件输出或超时本身 MUST NOT 被当作成功。

#### Scenario: Kernel banner appears before termination
- **WHEN** 构建成功且 QEMU 在限定时间内输出内核启动横幅
- **THEN** 验证记录横幅并返回成功，说明随后主动结束运行

#### Scenario: Only firmware output is visible
- **WHEN** QEMU 只输出固件信息且在限时内未出现内核横幅
- **THEN** 验证返回非零状态并保留实际输出供分析

### Requirement: Verify observed startup state
GDB 验证 MUST 报告复位 PC、内核入口、启动栈设置结果和 C 初始化入口，并将栈指针与本次 ELF 中的栈顶符号比较。连接失败、关键断点未命中或符号比较失败时 MUST 返回非零状态；证据 MUST 区分观察入口与完整单步固件。

#### Scenario: Stack matches the linked symbol
- **WHEN** 调试器命中内核入口并执行栈初始化指令
- **THEN** 输出设置前后 PC/sp 和 ELF 栈顶符号，并验证设置后的 sp 与符号地址相同

#### Scenario: Debug connection fails
- **WHEN** 调试连接不可用或限定时间内未命中所需入口
- **THEN** 验证有限时长退出，报告失败位置而非标记通过

### Requirement: Bound debug resources
自动运行 MUST 只使用本机回环调试连接，并在成功、失败和超时时清理自己启动的 QEMU/GDB 进程。MUST NOT 终止其他用户或其他会话的调试进程。

#### Scenario: A debugger times out
- **WHEN** 本次 GDB 验证超过时间限制
- **THEN** 本次启动的进程被清理且验证返回失败，其他会话保持运行

### Requirement: Keep evidence reproducible and truthful
验证材料 MUST 包含命令、工具版本、代码版本和本次观察范围。文本日志 MUST NOT 被称为真实终端截图；缺失评分环境或未读取完整题目时 MUST NOT 声称评分通过或全部练习完成。

#### Scenario: Grading script is missing
- **WHEN** 当前 Lab1 缺少课程评分脚本
- **THEN** 材料明确记录评分未执行，并分别报告实际构建、启动和调试结果
