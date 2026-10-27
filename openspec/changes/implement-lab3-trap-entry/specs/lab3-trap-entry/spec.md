## ADDED Requirements

### Requirement: Trap entry passes the saved frame to the C handler

`__alltraps` MUST 在已有 `SAVE_ALL` 建立 Trap frame 后，按照 RISC-V 调用约定通过 `a0` 将当前 `sp` 作为 `struct trapframe *` 传给 `trap()`，并在 `trap()` 返回后进入已有 `__trapret`。

#### Scenario: Enter the C trap handler

- **WHEN** 已委托给 S 模式的 Trap 跳转到 `__alltraps`
- **THEN** `SAVE_ALL` 保存现场，`trap()` 收到指向该 Trap frame 的 `a0`，且调用前后栈指针保持可恢复状态

### Requirement: Trap return restores the saved execution context

`RESTORE_ALL` MUST 按 `SAVE_ALL` 的槽位布局恢复 `sstatus`、`sepc`、`badvaddr`、`x1`、`x3`～`x31` 和原始 `sp`，随后已有 `sret` MUST 能返回原执行流。

#### Scenario: Restore registers without losing the frame base

- **WHEN** `trap()` 返回并进入 `__trapret`
- **THEN** 所有依赖当前 Trap frame 地址的读取在恢复 `sp` 前完成，原始 `sp` 最后恢复，各通用寄存器与 Trap 前保持一致

#### Scenario: Resume at the handler-selected EPC

- **WHEN** C handler 保留或修改 Trap frame 中的 `epc`
- **THEN** `RESTORE_ALL` 将当前 `tf->epc` 写入 `sepc`，不自行添加异常指令偏移；在 handler 正确推进断点 EPC 后不重复执行同一断点

### Requirement: Existing interfaces and scope remain unchanged

实现 MUST NOT 修改 `struct trapframe`、`trap()` 签名或其他函数和文件，且 MUST 保留 `lab3` 注释。`SAVE_ALL` MUST 保持既有行为，唯一允许的改动是经用户明确授权将旧版 `sbadaddr` CSR 名称替换为等价的现代 `stval` 名称，使当前汇编器能够编译。本 change MUST NOT 新增重复测试或用临时 handler 规避练习 2。

#### Scenario: Inspect the exercise 1 diff

- **WHEN** 审阅本 change 的实验代码 diff
- **THEN** 实验代码只有 `trapentry.S` 被修改，包含两个 `lab3 function1` 标记位置的实现，以及 `SAVE_ALL` 和 `RESTORE_ALL` 中各一处 `sbadaddr` → `stval` 兼容替换，没有其他行为变化

### Requirement: Exercise 1 is verified at the available integration level

`trapentry.S` MUST 能独立编译。完整寄存器恢复、断点返回、QEMU 和现有 grade MUST 在练习 2 合入后复用现有测试验证，本 change 不提前声称这些集成场景已通过。

#### Scenario: Compile the assembly implementation

- **WHEN** 运行 `make -C lab3 obj/kern/trap/trapentry.o`
- **THEN** 目标文件构建成功

#### Scenario: Complete handlers later

- **WHEN** 两个 Lab3 handler 已合入
- **THEN** 团队运行现有 build、QEMU 和 grade，检查寄存器上下文、异常返回和持续时钟中断
