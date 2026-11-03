## 1. Implementation

- [x] 1.1 在 `RESTORE_ALL` 中按 Trap frame 布局恢复 `sstatus`、`sepc`、`badvaddr` 和通用寄存器，最后恢复原始 `sp`
- [x] 1.2 在 `__alltraps` 中通过 `a0` 传递当前 Trap frame 并调用 `trap()`
- [x] 1.3 确认功能实现只修改两个 `lab3 function1` 标记位置，保留接口和其他代码
- [x] 1.4 按用户补充授权，仅将 `SAVE_ALL` 和 `RESTORE_ALL` 中旧版 `sbadaddr` 名称替换为 `stval`，不改变布局和行为

## 2. Verification

- [x] 2.1 运行 `make -C lab3 obj/kern/trap/trapentry.o`，确认汇编独立编译通过
- [x] 2.2 静态核对 `x1`、`x3`～`x31` 均恢复一次，CSR 偏移正确，`sp` 最后恢复，`a0` 在调用前取当前 `sp`
- [x] 2.3 尝试完整构建并记录练习 2 的既有阻塞，不通过临时 handler 绕过
- [x] 2.4 运行治理检查、OpenSpec 严格校验、`git diff --check` 并审阅最终 diff
- [x] 2.5 清理构建产物，记录完整 QEMU 和现有 grade 需在练习 2 合入后运行
