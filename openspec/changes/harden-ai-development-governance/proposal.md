# Proposal

## Why

现有 AI 开发规范已经要求先规格后实现，但在授权阶段、当前章节范围、跨工具技能路径和验证方式上存在互相冲突或无法执行的表述。若在正式实验开发前不收敛这些规则，不同 Agent 会对同一请求采取不同流程，且无法用自动检查证明规范得到遵守。

## What Changes

- 建立统一的 AI 开发状态机，明确 proposal、apply、验证、archive 和提交各阶段的授权边界。
- 把“只改当前章节”限定为实验实现代码，并明确 OpenSpec、Prompt 记录和获准的仓库级文档属于工作流元数据例外。
- 提供机器可读的当前章节和冻结章节状态，禁止修改已经冻结的章节。
- 使用平台原生的 OpenSpec skill 或命令，不再让 Claude、Cursor、OpenCode 和 Hermes 硬编码读取 Codex 的 `.agents/skills/` 路径。
- 为当前章节补充构建、运行、超时和验收说明，并增加可重复执行的治理检查和 CI。
- 消除 `AGENTS.md` 与 `CLAUDE.md` 的全文复制，统一文本行尾，并把关键治理规则纳入主规格。
- 修正根 README 的规范，使其中关于自动检查的陈述必须与仓库实际配置一致。
- 本 change 不改变 Prompt 原文的保存、脱敏或提交策略；该策略等待“可提交证据 + 加密原文”方案单独确认。

## Capabilities

### New Capabilities

- `ai-development-governance`: 规定 AI 修改仓库时的阶段、授权、章节作用域、验证和跨工具一致性。

### Modified Capabilities

- `root-readme`: 增加根说明中的开发状态和自动检查陈述必须可由仓库内容验证的要求。

## Impact

- 仓库级指令：`AGENTS.md`、`CLAUDE.md`、`.cursor/rules/openspec.mdc`、`openspec/config.yaml`。
- 开发说明：`README.md`、当前章节的局部说明文件，以及机器可读的章节状态文件。
- 质量门禁：`.gitattributes`、治理检查脚本和 GitHub Actions workflow。
- OpenSpec：新增 `ai-development-governance` 主规格，并更新 `root-readme` 规格。
- 不修改实验内核实现，也不改变现有 Prompt 归档格式。
