# Proposal

## Why

成员已在本机完成 Lab1 练习 1 和练习 2 的手工操作，12 张真实截图和 3 份 GDB 日志保存在仓库外的 record_Lab1 中。需要核对图片内容、解释证据与结论的对应关系，并把说明及原始材料整理成一个可审阅的开发提交。

## What Changes

- 在 lab1/report/record_Lab1/ 新增说明文档、12 张原始截图、正式实验日志和独立的排查日志，保留图片与日志原始字节。
- 逐图说明来源、观察结果和报告用途，解释启动栈、伪指令、复位跳板、固件交接及观察点的覆盖范围。
- 为导入材料和本次实验副本生成 SHA-256 清单，核对实验副本与仓库对应的 21 个源码及构建文件。
- 根 README 仅增加材料入口链接；使用独立 Dev 分支创建一个本地 commit，并向用户提供 push 命令。

范围：当前章节的实验说明与证据、根目录材料链接，以及本 change 的 OpenSpec 留痕。

非目标：修改内核或构建逻辑、重做或编辑截图、实现既有 prepare-lab1-submission 提案、生成完整小组报告、归档规格、执行 push 或创建 PR。

## Capabilities

### New Capabilities

无。本变更仅整理已有实验的说明与证据，不引入内核行为；使用 skip_specs: true。

### Modified Capabilities

无。根目录说明继续遵守 root-readme 的层次与单章命令边界，Prompt 保存规则不变。

## Impact

- 新增 lab1/report/record_Lab1/，保留仓库外原始材料。
- 修改根 README 的材料入口，新增本 change 的规划、任务与直接授权 Prompt 证据。
- 依赖已有 Python、Git、OpenSpec 和图像查看工具；构建核对使用现有 RISC-V 工具链。
- 用户本次请求明确授权整理说明和创建 commit；push 由用户执行。既有未跟踪提案不包含在本次提交中。
