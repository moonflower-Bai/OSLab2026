# Spec Delta

## Purpose

根据用户请求的行为影响选择直接维护或完整 OpenSpec 流程，使局部 bug 修复与文档编辑无需额外规划和实施审批。授权绑定已确认的需求范围，实施和维护在范围内连续推进，仅对未获准的新需求或实质扩展重新确认，并保留明确的验证与交付边界。

## ADDED Requirements

### Requirement: Route before invoking a workflow

AI MUST 在调用 OpenSpec 工作流之前先检查明确实施授权和快速、直接修改请求，再判断任务是否改变规格契约、增加实验能力、涉及跨模块重构或调整治理策略。没有明确直接实施授权时，上述任务 MUST 走完整流程；范围明确、恢复既定行为的局部修复，以及不改变代码或治理契约的文档、LaTeX 内容或排版修改 MUST 默认直接处理。行数、文件扩展名和文件数量 MUST NOT 单独决定分流；混合请求 MUST 按其中需要完整流程的部分处理相关实现。

#### Scenario: A two-line bug restores expected behavior
- **WHEN** 用户要求修复局部错误，修复恢复已有预期且不改变规格契约
- **THEN** AI 直接修复并验证，不先调用 OpenSpec

#### Scenario: A short patch changes interrupt semantics
- **WHEN** 补丁仅有两行，但引入新的中断处理契约
- **THEN** 没有明确直接实施授权时，AI 使用完整 OpenSpec 流程

#### Scenario: A LaTeX report is revised
- **WHEN** 用户要求修订报告文字、公式或排版，未要求改变实验代码或治理策略
- **THEN** AI 直接编辑并进行适当的文档验证，不创建 change

### Requirement: Treat a maintenance request as implementation authority

直接处理路径 MUST 将用户当前修改请求视为该范围的实施授权，MUST NOT 要求用户再发 apply 请求，MUST NOT 强制生成 proposal、specs、design 或 tasks。AI MUST 在完成后说明修改及验证结果。用户明确指定 OpenSpec 提案或流程时，AI MUST 遵从该指定；仅在明确修改需求时提及 OpenSpec 工具名称 MUST NOT 自动触发完整流程。

#### Scenario: The user asks to correct a typo
- **WHEN** 用户明确要求修改文档中的笔误
- **THEN** AI 在本轮完成编辑和必要验证，不等待第二次实施批准

#### Scenario: The user explicitly asks for a proposal
- **WHEN** 用户明确要求为一个小修复创建 OpenSpec 提案
- **THEN** AI 使用提案工作流并遵守其规划边界

### Requirement: Preserve staged authorization for substantial changes

完整流程 MUST 区分 Propose、Apply、Archive、Commit 的授权范围：新需求尚无明确实施批准时，提案完成后等待实施批准；已有适用批准或明确快速、直接修改请求时 MUST NOT 强制等待下一条消息。只有实施授权时 MUST NOT 自动归档或提交。归档和提交 MUST 有明确授权，但用户 MUST 能在一条指令中分别授权多个操作，AI MUST NOT 把操作边界解释为必须逐轮批准。对于已获得且仍适用的授权，AI MUST NOT 再次索取；用户明确停止或撤回时 MUST 遵从。已授权 change 范围内的局部实现修复 MUST NOT 为同一行为契约重新创建提案。

#### Scenario: A new experiment feature is requested
- **WHEN** 用户要求新增实验能力且没有适用的实施批准或快速、直接修改指令
- **THEN** AI 完成对应规划后等待明确实施授权

#### Scenario: A fix follows an authorized implementation
- **WHEN** 已授权 change 的实现存在局部错误，修复不改变其需求和设计范围
- **THEN** AI 复用该 change 修复并验证，不新建规划，不自动归档或提交

#### Scenario: Several operations are explicitly authorized together
- **WHEN** 用户明确要求应用已审阅 change、验证成功后归档并提交
- **THEN** AI 在各操作的条件满足后执行已授权步骤，不因到达下一阶段重复询问；普通实施批准不自动包含这些操作

### Requirement: Continue within the approved requirement scope

实施批准 MUST 覆盖为完成已确认需求所必要的实现、局部设计调整、bug 修复、失败验证后的修复、相关文档和既有规划产物同步。未改变用户可见行为契约、验收标准或实质范围时，AI MUST 连续推进，MUST NOT 因切换任务、编辑另一个已涉及文件、进入后续对话轮次或维护规划一致性反复请求批准。未出现明确撤回或停止时，授权 MUST 持续有效；没有授权的规划状态 MUST NOT 被视为已授权实施。

#### Scenario: Validation fails during approved implementation
- **WHEN** 验证发现已批准实现中的错误，修复仍满足原需求
- **THEN** AI 修复并重新验证，不先要求用户批准修复方案

#### Scenario: Implementation requires a local design adjustment
- **WHEN** 达成已批准需求需要调整局部实现方式并同步 design 和 tasks，行为契约和验收标准不变
- **THEN** AI 同步既有产物并继续实现，不为每个产物或任务单独等待批准

#### Scenario: Work resumes in a later turn
- **WHEN** 用户要求继续已批准 change，未扩大需求或撤回授权
- **THEN** AI 沿用已有批准推进剩余工作，不再要求一次 apply 批准

### Requirement: Confirm only unapproved requirement increments

AI MUST 对新增功能、改变行为契约或验收标准、实质扩大范围等未获准的新要求说明增量及其影响并获得确认，MUST NOT 把恢复既定行为、必要的实现细节调整或纯维护视为新要求。确认请求 MUST 仅针对未获准增量，且 MUST 在依赖该增量的实施开始前提出；不依赖该增量的已获准工作 MUST 继续推进。用户直接、明确要求追加并实施具体增量时，AI MUST 将其视为该增量的授权，不再就相同内容索取确认；用户仅讨论或要求提案时 MUST 保留规划边界。

#### Scenario: The agent discovers an optional new feature
- **WHEN** AI 在修复期间提出原批准范围外的可选功能
- **THEN** AI 说明该增量并等待批准，不自行实施该功能，继续原范围内独立工作

#### Scenario: The acceptance criterion would change
- **WHEN** AI 为了让测试通过准备放宽已确认的验收标准
- **THEN** AI 在修改标准前说明影响并等待批准，不能将其当作普通测试维护

#### Scenario: The user explicitly authorizes a concrete increment
- **WHEN** 用户明确要求追加并直接实施具体需求
- **THEN** AI 记录该授权，同步产物并执行增量，不再次询问是否批准同一追加需求

### Requirement: Honor explicit approval and quick edit requests

用户直接给出的明确实施批准，或针对具体范围的快速、直接修改请求 MUST 优先于默认规划和二次审批要求。AI MUST 在该范围内直接编辑和验证，已有 change 时复用并按需同步记录，无 change 时 MUST NOT 强制创建规划四件套才能实施。MUST NOT 把仅讨论审批策略、引用他人批准、含糊表态或仅要求提案视为实施批准；快速指令 MUST NOT 自动包含范围外功能、归档、提交或外部权限授权。

#### Scenario: Approval is included in the initial request
- **WHEN** 用户在具体需求中明确批准直接实施
- **THEN** AI 完成该范围的修改和验证，不要求用户另发一次 apply

#### Scenario: The user requests a quick change
- **WHEN** 用户明确要求快速或直接修改具体内容
- **THEN** AI 直接修改，不先生成规划四件套或停止等待批准

#### Scenario: Only a proposal is requested
- **WHEN** 用户明确只要求提案或讨论审批规则，没有授权实施
- **THEN** AI 只做该请求允许的规划或讨论，不把出现“批准”一词当成批准实施

### Requirement: Keep scope and verification on both routes

两条路径 MUST 遵守当前章节和冻结章节限制，以及适用于目标文件的验证要求。直接处理 MUST NOT 成为绕过章节保护的理由；遇到缺失的外部工具链 MUST 明确报告未执行的检查。需求歧义 MUST 只在影响实施结果时请求必要澄清，MUST NOT 因无法判断是否为“小改”就自动创建大量规划。

#### Scenario: A small fix touches a frozen chapter
- **WHEN** 局部修复需要修改已冻结章节
- **THEN** AI 停止该修改并报告范围冲突

### Requirement: Avoid creating changes solely for prompt evidence

无 change 的直接处理 MUST NOT 保存用户 Prompt，MUST NOT 为了保存 Prompt 人为创建 change 或新的会话档案。有对应 change 时，Prompt 证据 MUST 继续仅限人工确认相关且不含敏感值的原文，并保存在该 change 的 prompt.md；MUST NOT 读取其他 change 的证据。

#### Scenario: A standalone document edit has no change
- **WHEN** 用户要求直接修改 LaTeX 文档且没有对应 change
- **THEN** AI 不创建 Prompt 证据或 OpenSpec change

### Requirement: Keep routing guidance consistent across entry points

根规则、OpenSpec 配置、README、平台引导及各平台 OpenSpec 技能和命令正文 MUST 对分流和持续授权给出一致结论，且 MUST 引用根规则作为唯一授权标准。提示词 MUST NOT 忽略同一请求中的明确实施批准，或要求已批准范围内每次编辑、修复都等待批准。治理检查 MUST 接受合法的无 change 维护，并发现恢复旧审批要求的已知回退；生成工具更新覆盖本地调整后 MUST 能用仓库同步入口恢复。

#### Scenario: Different agents handle the same maintenance request
- **WHEN** 不同平台的 AI 收到恢复既定行为的局部修复请求
- **THEN** 各平台入口均允许直接处理，不因重复规则要求完整规划

#### Scenario: A native command receives an explicit quick edit request
- **WHEN** 用户通过 Claude、Cursor、OpenCode、Hermes 或共享技能入口要求快速修改具体内容
- **THEN** 命令和技能正文遵从根规则直接处理，不被旧的固定暂停条款阻断

#### Scenario: OpenSpec update restores generated prompts
- **WHEN** OpenSpec 更新把提示词恢复为默认版本
- **THEN** 提示词检查发现回退，仓库同步入口能恢复适用的授权条款
