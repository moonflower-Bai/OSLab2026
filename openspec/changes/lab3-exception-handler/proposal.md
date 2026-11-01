# Proposal

## Why

Lab3 的 `exception_handler` 仍是空函数，`trapentry.S` 在保存现场后没有进入 `trap()`。异常无法按 cause 处理，也无法从陷阱入口返回到 C 语言分派。

## What Changes

- 在 `lab3/kern/trap/trap.c` 实现 `void exception_handler(struct trapframe *tf)`：处理断点与非法指令，未知异常先打印 trapframe，是否 panic 由 `trap_in_kernel(tf)` 决定。
- 在 `lab3/kern/trap/trapentry.S` 只补上 `SAVE_ALL` 与 `RESTORE_ALL` 之间进入 `trap()` 的缺口，不在汇编里区分中断和异常。
- 不实现 `interrupt_handler`。中断仍由已有的 `trap_dispatch` 转交。

## Capabilities

### New Capabilities

- `lab3-exception-handling`: S 态陷阱进入 `trap()` 之后，断点、非法指令和未知异常的可观察结果。

### Modified Capabilities

- 无。

## Impact

- 只修改 `lab3/kern/trap/trap.c` 与 `lab3/kern/trap/trapentry.S`。
- `trap()`、`trap_dispatch`、`print_trapframe`、`trap_in_kernel`、`SAVE_ALL`、`RESTORE_ALL` 和 `interrupt_handler` 的现有行为保持不变。
- 验收命令是 `bash lab3/test/exception.sh`。断点、非法指令和未知异常的通过条件以该测试的宿主机层为准。

## Scope

- 实现异常处理函数，并让 `__alltraps` 在保存现场后以当前栈指针调用 `trap()`。
- 断点与非法指令打印对应地址并按指令长度推进 `epc`；未知异常打印 trapframe，内核态 panic，用户态返回。

## Non-goals

- 不实现、不补全 `interrupt_handler`。
- 不修改 `SAVE_ALL` 与 `RESTORE_ALL` 的宏体。
- 不在 `trapentry.S` 中做中断与异常分流。
- 不修改 `lab3/` 以外的实验代码，也不修改 `lab3/test/`。
- 不归档、不提交。
