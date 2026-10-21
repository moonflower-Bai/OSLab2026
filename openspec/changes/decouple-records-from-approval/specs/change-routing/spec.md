# Spec Delta

## MODIFIED Requirements

### Requirement: Route before invoking a workflow

AI MUST 在修改任何文件之前分别作出两项判断：是否需要 change 记录，以及是否需要暂停等待批准。是否需要 change MUST 只由需求影响决定：新增实验能力、改变规格契约或验收标准、跨模块重构、调整治理策略 MUST 有对应 change；范围明确、恢复既定行为的局部修复，以及不改变代码或治理契约的文档、LaTeX 内容或排版修改 MUST 默认直接处理而不创建 change。是否暂停 MUST 只由授权决定，授权 MUST NOT 改变是否需要 change 的结论。无法确定任务是否属于直接处理时，AI MUST 创建 change。行数、文件扩展名和文件数量 MUST NOT 单独决定分流；混合请求 MUST 按其中需要 change 的部分处理相关实现。

#### Scenario: A two-line bug restores expected behavior
- **WHEN** 用户要求修复局部错误，修复恢复已有预期且不改变规格契约
- **THEN** AI 直接修复并验证，不先调用 OpenSpec

#### Scenario: A short patch changes interrupt semantics
- **WHEN** 补丁仅有两行，但引入新的中断处理契约
- **THEN** AI 创建对应 change，是否暂停仅取决于用户是否已授权实施

#### Scenario: A LaTeX report is revised
- **WHEN** 用户要求修订报告文字、公式或排版，未要求改变实验代码或治理策略
- **THEN** AI 直接编辑并进行适当的文档验证，不创建 change

#### Scenario: A new experiment feature is requested with direct authority
- **WHEN** 用户要求“直接实现” Lab2 的新物理内存分配算法
- **THEN** AI 仍创建对应 change，而不是因为存在实施授权跳过记录

#### Scenario: The route is uncertain
- **WHEN** AI 无法确定一项实验代码修改是恢复既定行为还是引入新行为
- **THEN** AI 创建 change 记录该修改

### Requirement: Preserve staged authorization for substantial changes

需要 change 的工作 MUST 区分 Propose、Apply、Archive、Commit 的授权范围。尚无明确实施批准时，AI MUST 在完成规划产物后等待实施批准。已有适用批准或明确快速、直接实施请求时，AI MUST 先创建或复用 change，完成 apply 所需的全部产物及相关 Prompt 证据，运行该 change 的严格校验并修正发现的问题，然后在同一轮继续实施，MUST NOT 以提案完成为由等待下一条消息，也 MUST NOT 在产物缺失或校验失败时开始实施。只有实施授权时 MUST NOT 自动归档或提交。归档和提交 MUST 有明确授权，但用户 MUST 能在一条指令中分别授权多个操作，AI MUST NOT 把操作边界解释为必须逐轮批准。对于已获得且仍适用的授权，AI MUST NOT 再次索取；用户明确停止或撤回时 MUST 遵从。已授权 change 范围内的局部实现修复 MUST NOT 为同一行为契约重新创建提案。

#### Scenario: A new experiment feature is requested
- **WHEN** 用户要求新增实验能力且没有适用的实施批准或快速、直接修改指令
- **THEN** AI 完成对应规划后等待明确实施授权

#### Scenario: A new experiment feature is requested for direct implementation
- **WHEN** 用户要求新增实验能力并明确要求直接实现
- **THEN** AI 先完成 proposal、delta spec、tasks 等必要产物并通过严格校验，随后在同一轮实施和验证，不等待另一条 apply 消息

#### Scenario: Self-check fails before implementation
- **WHEN** 已授权实施，但 change 的严格校验失败
- **THEN** AI 先修正产物并重新校验，校验通过前不修改实验代码

#### Scenario: A fix follows an authorized implementation
- **WHEN** 已授权 change 的实现存在局部错误，修复不改变其需求和设计范围
- **THEN** AI 复用该 change 修复并验证，不新建规划，不自动归档或提交

#### Scenario: Several operations are explicitly authorized together
- **WHEN** 用户明确要求应用已审阅 change、验证成功后归档并提交
- **THEN** AI 在各操作的条件满足后执行已授权步骤，不因到达下一阶段重复询问；普通实施批准不自动包含这些操作

### Requirement: Honor explicit approval and quick edit requests

用户直接给出的明确实施批准，或针对具体范围的快速、直接修改请求，MUST 取消该范围内提案后的审批暂停，MUST NOT 取消该范围所需的 change 记录。对于需要 change 的范围，AI MUST 按产物、自检、实施的顺序在同一轮完成；对于直接处理的范围，AI MUST 直接编辑和验证，MUST NOT 为其创建规划产物。MUST NOT 把仅讨论审批策略、引用他人批准、含糊表态或仅要求提案视为实施批准；快速指令 MUST NOT 自动包含范围外功能、归档、提交或外部权限授权。

#### Scenario: Approval is included in the initial request
- **WHEN** 用户在一个需要 change 的具体需求中明确批准直接实施
- **THEN** AI 完成并自检 change 产物后实施和验证，不要求用户另发一次 apply

#### Scenario: The user requests a quick maintenance edit
- **WHEN** 用户明确要求快速修改文档笔误或恢复既定行为的局部错误
- **THEN** AI 直接修改，不生成规划产物或停止等待批准

#### Scenario: Only a proposal is requested
- **WHEN** 用户明确只要求提案或讨论审批规则，没有授权实施
- **THEN** AI 只做该请求允许的规划或讨论，不把出现“批准”一词当成批准实施

### Requirement: Keep routing guidance consistent across entry points

根规则、OpenSpec 配置、README，以及团队实际使用的 harness（Claude Code、Codex、DeepSeek harness、Hermes）所读取的 OpenSpec 技能和命令正文 MUST 对分流和持续授权给出一致结论，且 MUST 引用根规则作为唯一授权标准。提示词 MUST NOT 忽略同一请求中的明确实施批准，MUST NOT 要求已批准范围内每次编辑、修复都等待批准，也 MUST NOT 把实施授权表述为可以跳过需要的 change 记录。治理检查 MUST 接受合法的无 change 维护，并发现恢复旧审批要求或“授权即可跳过 change”的已知回退；生成工具更新覆盖本地调整后 MUST 能用仓库同步入口恢复。

#### Scenario: An unused tool keeps outdated prompts
- **WHEN** Cursor 或 OpenCode 的适配文件仍保留旧措辞
- **THEN** 治理检查不要求同步这些已不维护的入口

#### Scenario: Different agents handle the same maintenance request
- **WHEN** 不同平台的 AI 收到恢复既定行为的局部修复请求
- **THEN** 各平台入口均允许直接处理，不因重复规则要求完整规划

#### Scenario: A native command receives a direct feature request
- **WHEN** 用户通过 Claude Code、Codex、DeepSeek harness 或 Hermes 要求直接实现新实验功能
- **THEN** 命令和技能正文要求先完成并自检 change 产物再实施，不被旧的固定暂停条款阻断

#### Scenario: OpenSpec update restores generated prompts
- **WHEN** OpenSpec 更新把提示词恢复为默认版本
- **THEN** 提示词检查发现回退，仓库同步入口能恢复适用的授权条款
