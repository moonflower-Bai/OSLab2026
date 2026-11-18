# Proposal

## Why

Lab4 功能模块一需要在实现前建立可重复的验证：既检查叶项与物理页生命周期，也检查修改页表后处理器是否仍使用旧翻译。

## What Changes

- 在 `lab4/test/` 新增宿主机单元测试、独立启动的 QEMU Sv39 访存测试和固定种子的模型仿真测试。
- 提供顺序运行三层并汇总退出状态的独立脚本，缺少接口实现时报告链接诊断。
- 在测试临时副本中验证同页引用多增、末次删除漏回收、换页遗漏 TLB 失效这三种变体都能被检出。

## Capabilities

### New Capabilities

- `lab4-vmem-tests`: Lab4 `page_insert()` 和 `page_remove()` 的独立三层测试及失败诊断。

### Modified Capabilities

无。

## Impact

只新增 `lab4/test/` 下的测试和本 change 的记录。不会实现或修改内核接口，不修改主 Makefile、自检或评分规则，不读取 `lab4/tools/`，也不调用既有构建和评分入口。运行需要 Bash、Python 3、宿主机 C 编译器、RV64 裸机交叉编译器和 QEMU。
