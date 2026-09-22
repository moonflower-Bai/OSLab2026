# Proposal

## Why

用户发给 AI 的原话现在只留在各家聊天记录里，换一个人、换一个 harness 就对不上。需要一份按人分开、每次只含一条提示的仓库记录，并且启动时不要把历史提示读进上下文。

## What Changes

- 在各 harness 启动时自动读入的说明里增加一条：处理用户消息前，先把该条用户 Prompt 原文写入 `prompts/`。
- 规定目录按作用域、作者、日期拆开，一条提示一个文件，互不覆盖。
- 规定 AI 不得为了“找上下文”去列举或阅读 `prompts/` 里的其他文件。

## Capabilities

### New Capabilities

- `prompt-archive`: 保存单条用户 Prompt，并限制它不会进入后续会话的自动上下文。

### Modified Capabilities

- 无。`openspec/specs/` 里还没有任何规格。

## Impact

- `AGENTS.md`、`CLAUDE.md`、`.cursor/rules/openspec.mdc`：启动时读入的说明。
- 新增 `prompts/README.md` 和第一条按该结构保存的提示。
- `README.md`、`openspec/config.yaml`：让人和后续 OpenSpec 变更看到同一条约定。
