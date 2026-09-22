# Design

## Context

启动时自动读入的文件是 `AGENTS.md`（Codex、OpenCode、Hermes、DeepSeek Harness）、与它内容相同的 `CLAUDE.md`（Claude Code），以及始终生效的 `.cursor/rules/openspec.mdc`。`openspec/specs/` 目前为空。见 proposal.md 的 Why。

## Goals / Non-Goals

**Goals:**

- 一条用户 Prompt 对应一个不会和别人冲突的新文件。
- 文件本身能说明作者、时间和作用域。
- 历史提示留在仓库里，但不进入下一次会话的自动上下文。

**Non-Goals:**

- 不保存助手回复、工具输出或完整对话。
- 不把 `prompts/` 放进 `.cursorignore` 或 `.gitignore`。忽略目录会妨碍写入，也不能替代“不要去读”这条说明。

## Decisions

### 路径按作用域、作者、日期分开

`prompts/<scope>/<author>/<YYYY-MM-DD>/<HHMMSS>-<slug>.md`

- `<scope>` 是 `lab1`、`lab2` 这类章节名。不针对某一章时用 `repo`。
- `<author>` 取 `git config user.name`，转成小写并把空格换成连字符，去掉路径分隔符。读不到时用 `unknown`，不猜测真名。
- `<HHMMSS>` 用本地时间，避免同一天同一作者的文件名相撞。
- `<slug>` 用三到六个英文单词概括，不把原文放进文件名。

一人一个目录，所以三个人同时写不会改到同一个文件。唯一的共享文件是 `prompts/README.md`。

备选是按人只放一个追加日志。那会让合并冲突，也会让一次读文件带进该作者的全部历史。

### 文件只放元数据和原文

正文前用三行元数据：`author`、`authored_at`（ISO 8601）、`scope`。其后空一行，再放用户原文。不含助手回复。

若原文里有 API Key、token 或私钥，保存前替换成 `<redacted>`。

### 启动说明只规定写入，不规定读回

`AGENTS.md` 与 `CLAUDE.md` 保持逐字相同，这样 DeepSeek Harness 只会注入一次。Cursor 规则太短，单独写上“先写入、不读其他提示”，避免它只指向 `AGENTS.md` 时被漏掉。

允许阅读的只有 `prompts/README.md` 和刚写完的那一个文件。不在启动说明里粘贴任何历史提示。

## Risks / Trade-offs

- [作者名来自本机 git 配置，两人配置成同一个名字会写进同一目录] → 文件名还有时分秒；仍冲突时改用更长的时间戳，不覆盖已有文件。
- [模型可能仍然主动去翻 `prompts/`] → 启动说明写成禁止列举和搜索，而不是依赖忽略文件。
- [密钥被写进提示] → 保存前替换明显的密钥，原文其余部分保持不变。

## Migration Plan

新增目录和说明即可。没有旧的 `prompts/` 需要迁移。若要撤掉，删掉启动说明里的这一节和 `prompts/` 目录。
