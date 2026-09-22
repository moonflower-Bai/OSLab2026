# Spec Delta

## MODIFIED Requirements

### Requirement: Save each user prompt
AI MUST 只保存与某个 OpenSpec change 的项目实现直接相关且不含敏感信息的用户 Prompt。符合条件的 Prompt MUST 原样追加到该 change 的 `prompt.md`；普通问答、状态询问和与实现无直接关系的消息 MUST NOT 保存。

#### Scenario: A new user message arrives
- **WHEN** 会话收到一条新的用户消息
- **THEN** AI 在写文件前判断它是否直接影响 change 且不含敏感信息，只在两个条件都满足时保存原文

#### Scenario: A prompt directly changes the project
- **WHEN** 用户消息创建、修改或应用一个 change，或改变其需求、设计、任务或验收条件，且消息不含敏感信息
- **THEN** 该消息原文被追加到对应 change 的 `prompt.md`

#### Scenario: A prompt is not implementation evidence
- **WHEN** 用户消息只是普通问答、状态询问或与项目实现无直接关系
- **THEN** 仓库中不为该消息写入 Prompt 记录

### Requirement: Keep a prompt file self-contained
每个 change 的 `prompt.md` MUST 包含该 change 的标识、记录时间和所有已保存的用户原文，并 MUST NOT 包含助手回复、工具输出或其他 change 的 Prompt。

#### Scenario: Someone opens one prompt file later
- **WHEN** 只打开某个 change 的 `prompt.md`
- **THEN** 不借助聊天记录也能看出这些原文属于哪个 change 以及各自记录时间

#### Scenario: Someone audits an assignment change
- **WHEN** 审阅者只打开该 change 的 `prompt.md` 和其他 OpenSpec 产物
- **THEN** 审阅者能够对应原始实现 Prompt 与该 change 的需求、设计和任务

### Requirement: Do not load the archive into context
AI MUST NOT 为了获取上下文而列举、搜索或读取其他 change 的 `prompt.md` 或旧的会话归档。AI MAY 只读取当前已确定 change 的 `prompt.md` 以便追加或核对本次提交。

#### Scenario: A later session starts
- **WHEN** 新会话开始，仓库里已经存在其他 change 的 Prompt 证据或旧会话文件
- **THEN** 那些原文不会进入新会话的自动上下文

#### Scenario: A later task starts
- **WHEN** 仓库中已经存在其他 change 的 Prompt 证据或旧会话文件
- **THEN** 这些原文不会自动进入当前任务上下文

## REMOVED Requirements

### Requirement: Group one conversation into one file
**Reason**: 作业留痕的归档单位改为直接驱动实现的 OpenSpec change，而不是包含普通问答的完整会话。
**Migration**: 新 Prompt 写入对应 change 的 `prompt.md`；既有会话文件作为本地遗留内容保留且不再提交，不读取或迁移其他会话。

### Requirement: Resolve the conversation outside compressed context
**Reason**: 新路径由当前 OpenSpec change 决定，不再依赖外部会话号。
**Migration**: 在创建或选定 change 后保存合格 Prompt；没有 change 的消息不保存。

## ADDED Requirements

### Requirement: Reject sensitive prompts without redaction
包含姓名、学号、身份证明、账号凭证、访问令牌、私钥或其他敏感信息的用户消息 MUST NOT 以原文或脱敏副本写入仓库。若该消息与当前 change 直接相关，系统 MAY 只记录不含原文和敏感值的省略标记。

#### Scenario: An implementation prompt contains student identity
- **WHEN** 与实现相关的用户消息包含姓名或学号
- **THEN** `prompt.md` 不包含该消息原文或其局部改写，最多只记录该时间有一条敏感输入被省略

### Requirement: Gate submitted prompt evidence
仓库的自动检查 MUST 拒绝新提交的会话归档、不允许的 Prompt 路径、明显敏感信息模式和脱敏占位内容。自动检查 MUST 只报告失败，MUST NOT 自动改写 Prompt。

#### Scenario: Pull request contains a student number in prompt evidence
- **WHEN** pull request 新增或修改的 Prompt 证据匹配已配置的学号模式
- **THEN** CI 失败并报告文件位置，原文件保持不变

#### Scenario: Prompt passes automated patterns
- **WHEN** Prompt 证据未匹配自动检查的敏感模式
- **THEN** CI 仍把内容相关性和无法自动识别的敏感信息留给提交者及审阅者确认，不宣称内容必然安全
