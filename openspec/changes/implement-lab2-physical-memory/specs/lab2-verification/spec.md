# Spec Delta

## Purpose

规定 Lab2 在现有多章节仓库中的开发布局、构建运行和评分结果的记录方式，以及实验报告与课程交付预览应具备的可检查证据。该契约区分代码验证成功、报告准备和最终交付，防止用未运行的检查或不完整材料宣称实验完成。

## ADDED Requirements

### Requirement: Lab2 development remains isolated

Lab2 开发内容 SHALL 位于 `lab2/`，源码沿用基线目录结构，`report/` 保存报告材料；当前章节 SHALL 标识为 `lab2`。实施 Lab2 时 MUST 保留所选集成基线的内容边界和仓库外原始骨架。

#### Scenario: Preparing the development tree

- **WHEN** 导入 Lab2 骨架并准备实施
- **THEN** `lab2/` 可独立构建，相对集成基线未额外更改 Lab1，原始骨架仍可作为对照

### Requirement: Validation uses the supplied framework unchanged

Lab2 验证 SHALL 保留提供的自检函数及评分测试原文，并分别记录编译、启动和评分结果。缺少运行依赖、发生 panic 或只得到超时结果时 MUST NOT 标记验证通过。

#### Scenario: A successful validation run

- **WHEN** 在声明的环境中完成编译、启动和评分
- **THEN** 编译退出码为零，启动出现 `check_alloc_page() succeeded!`，评分出现两项 PASS 和 `Total Score: 20/20`

#### Scenario: An intentional idle loop reaches the external timeout

- **WHEN** 启动检查已输出预期成功标记，随后因内核空转被外部限时终止
- **THEN** 记录中分别说明启动成功的输出证据与限时结束原因，不以超时退出码单独推断成功

### Requirement: Report evidence reflects actual work

实验报告 SHALL 按提供模板记录已实现模块、原理说明、实际提示词迭代和测试结果。引用的图片 MUST 来自真实运行并能通过相对路径打开；未知的组员信息和未执行的检查 MUST 明确留待补充。

#### Scenario: Reviewing a module in the report

- **WHEN** 读者查看一个已完成模块
- **THEN** 可以找到该模块的实现说明、实际验证结果和对应证据，内容不包含编造的迭代、成员或通过记录

### Requirement: Submission readiness identifies unresolved delivery constraints

交付预览 SHALL 明确 `lab2/`、`lab2/report/` 到课程要求的根级 `code/`、`report/` 的映射。目录、提示词文件规则或指导书必做项尚未核对完成时，交付状态 MUST 保持未完成，且 MUST NOT 为生成预览放宽现有门禁。

#### Scenario: Prompt delivery conflicts with current repository rules

- **WHEN** 课程要求 `report/prompt.md`，现有规则仅允许 change 级提示词证据
- **THEN** 预览说明该冲突及待审阅的后续调整，保留当前 change 级证据，并且不声称已满足最终交付要求
