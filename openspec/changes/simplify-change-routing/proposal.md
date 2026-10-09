# Proposal

## Why

当前规则把所有仓库修改都要求先完成 OpenSpec 规划并另行授权实现，导致局部 bug 修复和 LaTeX 文档编辑也被阻塞。需要在调用工作流之前按行为影响分流，并让一次实施批准覆盖已确认需求的持续实现和维护，只在出现尚未批准的新需求或实质范围变化时重新确认。

## What Changes

- 增加直接处理与完整 OpenSpec 两条路径：局部修复、纯文档和 LaTeX 编辑默认直接处理；新增实验功能、改变规格契约、跨模块重构及治理策略调整走完整流程。
- 明确实施批准或快速、直接修改的请求优先于默认分流：按明确授权范围直接完成，不再强制新建规划、等待另一条 apply 消息；仅讨论或只要求提案的请求仍不授权实施。
- 直接处理以用户当前修改请求作为实施授权，不创建 change、不生成规划四件套、不追加 apply 审批；涉及已有 change 的实现细节修复可复用其记录。
- 批准绑定需求范围而非文件、任务或对话轮次；批准后连续完成实施、必要的局部设计调整、bug 修复、验证失败后的修复、文档及规划同步，不逐项停下等待批准。
- 仅对新增能力、行为契约或验收标准改变、实质范围扩大等未获准增量重新确认；确认请求说明新增内容和影响，已获准且不依赖增量的工作继续推进。明确停止或撤回授权时立即遵从。
- 按影响而非行数判断；改两行却改变中断语义仍走完整流程，修改大量排版仍可直接处理。明确需求或用户指定流程时遵从该指定。
- 保留当前章节、冻结章节、适当验证、敏感信息保护及明确的归档和提交授权；用户可一次明确授权多个操作，已有授权不重复索取。
- 直接处理且没有 change 时不保存 Prompt，也不为保存 Prompt 人为创建 change。
- 同步根规则、OpenSpec 配置、README、Claude/Cursor 入口和 Prompt 说明，并修改 Codex、Claude、Cursor、OpenCode、Hermes 的技能与命令提示词，消除正文中的重复审批条款；增加可重放的同步脚本与检查，避免 OpenSpec 更新后恢复旧逻辑。

范围仅为工作流治理、各平台提示词及其验证；不实现实验功能、不编辑具体 LaTeX 文档、不自动归档或提交。

## Capabilities

### New Capabilities

- `change-routing`: 规定何时直接处理，何时调用完整 OpenSpec，以及按需求范围持续授权、增量确认、留痕和验证的行为。

### Modified Capabilities

无。现有主规格没有分流能力；本次不扩展根 README 的结构要求或重做历史 Prompt 保存方案。

## Impact

- `AGENTS.md`、`openspec/config.yaml`、`README.md`、`.cursor/rules/openspec.mdc`、`prompts/README.md`。
- `.agents/skills`、`.claude/skills`、`.claude/commands/opsx`、`.cursor/skills`、`.cursor/commands`、`.opencode/skills`、`.opencode/commands`、`.hermes/skills` 共 48 份提示词；`CLAUDE.md` 保留原生导入根规则的单一入口。
- `scripts/check_governance.py` 的规则入口检查及自检和 `scripts/sync_ai_workflows.py`；现有 CI 继续复用同一入口。
- 增加 `change-routing` delta spec；保留已有完成但未归档 change 的历史记录，说明后续同步时不能恢复旧的一刀切要求。
- 不改变 CLI schema、不引入新依赖、不修改章节文件。
