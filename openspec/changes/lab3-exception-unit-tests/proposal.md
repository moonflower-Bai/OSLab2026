# Proposal

## Why

Lab3 的异常处理还没有可在宿主机上直接判定对错的单元测试。需要一套只覆盖异常路径的测试，能在不启动 QEMU 的情况下检查 `exception_handler` 的输出、`epc` 推进和 trapframe 不变式，并把依赖完整内核的保存/恢复与委托检查先写成可运行的 QEMU 测试、待实现合入后再执行。

## What Changes

- 新增 `lab3/test/exception/` 与 `lab3/test/exception.sh`。`bash lab3/test/exception.sh` 编译并运行宿主机测试，逐项输出「通过」或「错误」；失败时只输出当前错误值。
- 宿主机测试桩住 `interrupt_handler`、`panic` 和 `cprintf`，调用内核里的 `trap()` / `exception_handler`，不依赖 QEMU。
- 新增 QEMU 层测试源码，检查 trapframe 槽位、进入 `trap()` 时的 `sp` 与 `sscratch`、寄存器恢复，以及 S 态 `ebreak` 的委托行为。本变更不运行该层。
- 不实现 `interrupt_handler`，也不修改 Lab3 异常实现本身。

## Capabilities

### New Capabilities

- `lab3-exception-tests`: Lab3 异常处理单元测试的可观察结果，包括宿主机异常分派、`epc` 与 trapframe 不变式、失败输出，以及 QEMU 层保存/恢复和异常委托的通过条件。

### Modified Capabilities

- 无。

## Impact

- 新增文件限于 `lab3/test/exception.sh` 和 `lab3/test/exception/`。`lab3/` 以外的实验代码、内核实现和 `lab3/tools/` 保持不变。
- 宿主机测试链接当前 `lab3/kern/trap/trap.c`。`exception_handler` 尚未实现时，脚本以错误结束。
- QEMU 测试依赖完整内核（`trapentry.S` 的保存/恢复与 `exception_handler`）合入后才能执行。
- 测试不读取 `medeleg`，中断路径只使用桩。

## Scope

- 在 `lab3/test/` 增加异常单元测试及直接入口 `exception.sh`。
- 宿主机层覆盖断点、非法指令、按指令长度推进 `epc`、未知异常的 `print_trapframe` / `panic` 分支，以及除 `epc` 外 trapframe 字段不变。
- QEMU 层只编写：36 槽 trapframe、`trap()` 收到的指针、寄存器与 `sp` 恢复、进入陷阱时 `sscratch == 0` 且返回前恢复、S 态 `ebreak` 直接进入 S 模式。

## Non-goals

- 不实现或修改 `exception_handler`、`interrupt_handler`、`trapentry.S` 或其他内核代码。
- 不修改 `lab3/` 以外的文件（本 change 的 OpenSpec 元数据除外）。
- 不读取 `lab3/tools/`。
- 不在本变更中启动 QEMU 或运行 QEMU 层测试。
- 不把对抗用的错误 `exception_handler` 变体写入仓库。
