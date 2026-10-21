# Proposal

## Why

`simplify-change-routing` 把“是否停下等待批准”和“是否创建 change”绑在同一个判断上：完整 OpenSpec 只适用于“没有明确直接实施授权”的新实验功能，而实验请求通常本身就是实施授权，于是 AI 普遍把“直接实现”理解为可以跳过 OpenSpec，新功能（如 Lab2 best-fit）不再留下 proposal、delta spec 和 tasks。用户的本意是：直接实施授权只取消“提案后等待批准”的暂停，不取消规划记录；AI 应先完成 OpenSpec 产物并自检，再在同一轮直接实现。

## What Changes

- 拆分两个判断：**是否需要 change** 只由需求影响决定（新实验功能、行为契约或验收标准变化、跨模块重构、治理策略调整需要；恢复既定行为的局部修复、纯文档和 LaTeX 修改不需要）；**是否暂停** 只由授权决定（有实施授权时不暂停，否则在提案后等待批准）。
- 有实施授权且需要 change 时，AI MUST 先创建或复用 change，完成 proposal、delta spec、tasks（design 按 schema 条件）及相关 Prompt 证据，运行 `openspec validate <change> --strict` 自检通过，然后在同一轮直接实施，不等待另一条 apply 消息。
- 无法判断是否属于直接处理时，默认创建 change。
- 改写 `AGENTS.md`、`openspec/config.yaml`、`README.md`、`.cursor/rules/openspec.mdc` 中劝阻使用 OpenSpec 的措辞，使“先留记录、授权决定是否暂停”成为首要规则。
- 更新 `scripts/sync_ai_workflows.py` 的授权段落，并把同步范围收窄到团队实际使用的 harness：Claude Code（`.claude` 技能与命令）、Codex 与 DeepSeek harness（`AGENTS.md`、`.agents` 技能）、Hermes（`.hermes` 技能）。Cursor 与 OpenCode 的适配不再维护，现有文件保留不动。
- 更新 `scripts/check_governance.py` 的规则关键词与自检，拒绝“授权即可跳过 change”的回退措辞。

非目标：不增加 PR 级“实验代码必须伴随 change”检查（AI 事后补记录意义不大，靠规则在动手前生效）；不为 Lab2 已有工作（如 best-fit）补记录，从 Lab3 开始按新规则执行；不恢复每一阶段都要单独批准的旧流程；不改变归档、提交须明确授权的边界；不修改实验代码；不归档本 change 或 `simplify-change-routing`；不读取其他 change 的 Prompt 证据。

## Capabilities

### New Capabilities

无。

### Modified Capabilities

- `change-routing`：分流改为按影响决定是否需要 change、按授权决定是否暂停；明确授权下“先产物、后自检、再实施”；不确定时默认创建 change；规则入口只要求覆盖实际使用的 harness。该能力目前只存在于未归档的 `simplify-change-routing` delta 中，本 change 归档前须先归档该 change。

## Impact

- 规则入口：`AGENTS.md`（`CLAUDE.md` 仅导入它）、`openspec/config.yaml`、`README.md`、`prompts/README.md`（如措辞需要同步）。
- 平台提示词：`scripts/sync_ai_workflows.py` 及其重放的 `.agents`、`.claude`、`.hermes` 技能与 `.claude/commands/opsx` 命令。
- 治理检查：`scripts/check_governance.py`（关键词与自检更新）；CI 工作流无需改动。
- 不改变章节代码、OpenSpec CLI 版本或依赖。
