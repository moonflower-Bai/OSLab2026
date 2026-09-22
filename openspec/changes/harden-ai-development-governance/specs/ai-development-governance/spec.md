# Spec Delta

## Purpose

为不同 AI 编程工具规定同一套可验证的仓库修改流程，使规划、实现、验收、归档和提交具有明确授权，并确保实验代码只影响获准的当前章节。

## ADDED Requirements

### Requirement: Separate planning from implementation
AI 收到仓库修改请求时 MUST 先创建并完成对应的 OpenSpec 规划产物，且 MUST 在后续获得明确的 apply 请求后才修改规划产物以外的实现或规范文件。

#### Scenario: User requests a repository change
- **WHEN** 用户要求实现实验、生成内核代码或修改仓库规范，且尚无对应 change
- **THEN** AI 完成该 change 的规划产物并停止，不在同一轮开始实现

### Requirement: Require authority for irreversible workflow transitions
AI MUST 分别获得 apply、archive 和 Git commit 的明确授权；完成前一阶段 MUST NOT 自动取得后一阶段的授权。

#### Scenario: Implementation tasks are complete
- **WHEN** change 的实现和验证已经完成
- **THEN** AI 报告结果并等待 archive 授权，不自动归档或提交

### Requirement: Define the current and frozen chapters
仓库 MUST 提供一个机器可读的章节状态源，且其中 MUST 明确当前章节和已经冻结的章节。AI MUST NOT 修改冻结章节中的实验实现文件。

#### Scenario: A requested change touches a frozen chapter
- **WHEN** 计划或实际 diff 包含已冻结章节中的实验实现文件
- **THEN** 治理检查失败，AI 停止修改并报告冲突文件

### Requirement: Distinguish implementation scope from workflow metadata
“只改当前章节” MUST 约束实验实现文件，但 MUST 允许 change 已声明的 OpenSpec 产物、Prompt 记录、治理配置和仓库级文档作为工作流元数据被修改。

#### Scenario: Applying a current-lab change
- **WHEN** AI 实现当前章节的 change
- **THEN** 实验实现 diff 只位于当前章节，而获准的工作流元数据可以位于仓库级目录

### Requirement: Use platform-native workflow entry points
共享仓库规则 MUST 按语义名称引用 OpenSpec workflow，并 MUST NOT 要求非 Codex 工具读取 Codex 专用的 `.agents/skills/` 实现。

#### Scenario: Different supported agents apply the same change
- **WHEN** Claude、Codex、Cursor、OpenCode、Hermes 或 DeepSeek Harness 处理仓库修改
- **THEN** 每个工具使用自身可发现的 OpenSpec skill 或命令，同时遵守同一份仓库级状态机

### Requirement: Provide chapter-local verification instructions
当前章节 MUST 提供可直接执行的构建、运行边界和验收说明。AI 完成实现前 MUST 运行适用于改动的检查，或明确报告无法运行的原因。

#### Scenario: Current chapter files are changed
- **WHEN** change 修改当前章节的实验实现文件
- **THEN** AI 按该章节说明执行至少一个确定会终止的构建或检查命令，并报告结果

### Requirement: Enforce governance automatically
仓库 MUST 提供本地和 CI 可调用的治理检查，至少验证 OpenSpec 严格校验、冻结章节保护、关键规则入口和文本行尾。

#### Scenario: A pull request violates a governance invariant
- **WHEN** pull request 包含无效 OpenSpec、冻结章节修改、失效的规则入口或不合规行尾
- **THEN** 自动检查失败并指出对应问题
