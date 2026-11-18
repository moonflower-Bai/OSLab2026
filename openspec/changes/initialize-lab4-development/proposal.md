# Proposal

## Why

当前检出 Lab4 稳定分支且已有 `lab4/lab4_new/` 骨架，但章节状态仍指向不存在的 Lab3，根说明仍描述 Lab1。需要让开发状态与现有源码一致，并确认本机工具链可以构建和启动实验。

## What Changes

- 将当前章节切换为 `lab4`，保持冻结列表不变。
- 按用户后续要求保留当前 `Dev/Lab4/main` 分支，改动留在工作区，后续切换分支时一起提交。
- 更新根说明的章节进度，在 `lab4/README.md` 提供实际源码入口和构建、运行、评分说明。
- 检查现有工具链，执行构建、限时启动和现有评分命令，记录真实结果；若存在环境兼容问题，在不改变实验行为和验收的前提下解决。
- 范围不包含完成 Lab4 练习、改变评分规则、整理源码目录、冻结其他章节、提交、推送或归档。

## Capabilities

### New Capabilities

- `lab4-development`: 当前章节与 Lab4 源码一致，并提供可复现的开发环境使用说明及验证结果。

### Modified Capabilities

无；根说明沿用已有结构和要求，仅纠正实际进度。

## Impact

涉及 `lab-status.json`、`README.md`、`lab4/README.md` 和本 change 元数据；必要的环境兼容修正涉及当前 Lab4 的 Makefile 与 `tools/grade.sh` 中 QEMU 加载参数，保持评分断言和分数不变。使用已有 GCC、binutils、QEMU 和 Python，不预设需要安装新软件。实验功能缺失与环境故障分别报告，初始化不以完成实验练习为目标。
