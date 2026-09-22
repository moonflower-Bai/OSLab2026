# Proposal

## Why

作业提交需要保留能够解释实现来源的原始 Prompt，但按会话保存每条用户消息会把无关内容和姓名、学号等敏感信息带入 Git。Prompt 留痕应只覆盖直接驱动项目实现的非敏感原文，并在提交前阻止不合规内容。

## What Changes

- **BREAKING**：停止按会话把每条用户消息追加到 `prompts/<author>/<session-id>.md`。
- 与项目实现直接相关且不含敏感信息的用户 Prompt，原样保存在对应 OpenSpec change 的 `prompt.md`。
- 普通问答、状态询问和与实现无直接关系的消息不保存。
- 含姓名、学号、凭证或其他敏感信息的消息整条不保存，不生成脱敏副本；需要留痕时只记录不含原文的省略标记。
- GitHub Actions 调用仓库内检查脚本，阻止会话归档、明显敏感模式或不合规 Prompt 证据进入提交；检查失败，不自动改写内容。
- 将旧的 `prompts/` 会话文件视为本地遗留记录并从后续提交范围排除，不读取或迁移其他会话内容。

## Capabilities

### New Capabilities

- 无。

### Modified Capabilities

- `prompt-archive`: 把归档单位从整组会话改为与 OpenSpec change 关联的非敏感实现 Prompt，并增加提交门禁。

## Impact

- `AGENTS.md`、`CLAUDE.md`、`.cursor/rules/openspec.mdc`、`openspec/config.yaml`：改变消息到达时的保存触发条件和位置。
- `prompts/README.md`、`.gitignore`：停止提交按会话生成的明文记录，并说明遗留文件策略。
- `openspec/changes/<change>/prompt.md`：成为作业 Prompt 的提交位置。
- 治理检查脚本和 GitHub Actions：增加 Prompt 相关性、路径和敏感模式门禁。
- 不读取、重写或迁移其他会话的既有 Prompt 文件。
