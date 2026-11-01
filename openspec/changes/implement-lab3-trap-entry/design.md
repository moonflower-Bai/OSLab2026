## Context

`SAVE_ALL` 在栈上分配 36 个 `REGBYTES` 槽位，使当前 `sp` 指向 `struct trapframe`。槽位 0～31 对应通用寄存器，槽位 32～35 分别为 `sstatus`、`sepc`、`badvaddr`、`scause`。原始 `sp` 经 `sscratch` 和 `s0` 中转后保存在槽位 2，而不是保存分配 Trap frame 后的当前栈顶。

## Goals / Non-Goals

**Goals:**

- 将 Trap frame 按 RISC-V ABI 传给 `trap()`。
- 与 `SAVE_ALL` 对称地恢复执行上下文，且不在恢复过程中破坏读取基址。
- 除两处等价的 `sbadaddr` → `stval` 工具链兼容替换外，修改范围只在两个 `lab3 function1` 标记处。

**Non-Goals:**

- 不实现异常 EPC 推进或中断处理，不添加重复测试。

## Decisions

1. `__alltraps` 在 `SAVE_ALL` 后把当前 `sp` 写入 `a0`，再以 `jal trap` 调用 C handler。返回地址由 `jal` 设置，控制流自然落入已有 `__trapret`。
2. `RESTORE_ALL` 先以 `s0`、`s1`、`s2` 为中转，从槽位 32、33、34 恢复 `sstatus`、`sepc`、`badvaddr`。这些寄存器的原值随后再从 Trap frame 恢复。
3. 不恢复硬连线为零的 `x0`，不恢复仅描述本次 Trap 原因的 `scause`。`SAVE_ALL` 已将 `sscratch` 清零，本练习不扩展用户态栈切换行为。
4. 恢复 `x1`、`x3`～`x31` 后，最后从槽位 2 恢复 `sp`。在此之前所有加载均以当前 Trap frame 栈顶为基址。
5. `RESTORE_ALL` 使用 Trap frame 中当前的 `epc`；断点或非法指令是否推进 `epc` 由练习 2 的 handler 决定，汇编入口不自行添加固定偏移。
6. 先独立编译 `trapentry.o` 并静态核对寄存器覆盖和恢复顺序。完整运行时验证复用现有 grade，留待练习 2 合入后执行。

## Risks / Trade-offs

- 提前恢复 `sp` 会使后续加载使用错误基址 → 将 `sp` 恢复固定为最后一条上下文加载。
- 提前恢复 `s0`～`s2` 会破坏 CSR 中转值 → 先写 CSR，再恢复通用寄存器。
- 当前只能做独立编译，不能声称完整 Trap 闭环已通过 → 明确记录练习 2 的依赖，不用临时 handler 绕过。
