## Why

Lab3 练习 1 要求通过 spec/prompt 借助 AI 完成 Trap 上下文恢复与入口。当前 `trapentry.S` 已提供 `SAVE_ALL` 和 `__trapret`，但 `RESTORE_ALL` 与 `__alltraps` 仍为占位，无法把 Trap frame 交给 C handler，也无法恢复现场并返回。

## What Changes

- 在 `lab3/kern/trap/trapentry.S` 实现 `RESTORE_ALL`，按现有 36 槽位 Trap frame 恢复 CSR 和通用寄存器。
- 在 `__alltraps` 中通过 `a0` 传递当前 Trap frame 地址并调用 `trap()`。
- 保持 `struct trapframe`、`trap()` 签名及 `__trapret` 不变；除经用户明确授权将旧 CSR 名称 `sbadaddr` 兼容替换为 `stval` 外，不改变 `SAVE_ALL`。
- 使用现有构建和后续集成测试验收，不新增重复测试。

## Non-goals

- 不实现 `interrupt_handler()` 或 `exception_handler()`。
- 不修改其他函数、文件或 Trap frame 布局。
- 不新增测试文件，不归档、提交或推送（除非后续另行授权）。

## Capabilities

### New Capabilities

- `lab3-trap-entry`: 规定 Trap frame 从汇编入口传递到 C handler，以及处理完成后恢复上下文并通过 `sret` 返回的行为。

### Modified Capabilities

无。

## Impact

- 修改 `lab3/kern/trap/trapentry.S`。
- 新增本 change 的规划、规格和 Prompt 证据。
- 练习 2 尚未完成，完整内核、QEMU 和 grade 仍依赖后续 handler 实现。
