# Proposal

## Why

一条提示一个文件，既拆散了同一组对话，也让 AI 在上下文被压缩之后无法判断下一条该写入哪个文件。会话号必须来自压缩删不掉的进程环境，而不是来自聊天记忆。

## What Changes

- 同一组对话的用户 Prompt 追加进同一个文件。
- 每次保存前从环境变量或该环境指向的会话目录重新计算会话号。
- 没有外部会话号时新建文件，不追加到最近改过的对话上。
- 把已经按单条拆开的本对话记录并回一个文件。

## Capabilities

### New Capabilities

- 无。

### Modified Capabilities

- `prompt-archive`: 保存单位从单条提示改为由外部会话号确定的一组对话，并规定压缩之后如何判定所属对话。

## Impact

- `AGENTS.md`、`CLAUDE.md`、`.cursor/rules/openspec.mdc`、`prompts/README.md`、根目录 `README.md`、`openspec/config.yaml`。
- `prompts/` 下已有的单条文件合并进当前会话文件。
