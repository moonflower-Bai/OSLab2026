# Design

## Context

`prompts/` 里现在是一条提示一个文件。见 proposal.md 的 Why。上下文压缩会丢掉「上次写到哪个路径」这段聊天记忆，但不会丢掉进程环境和启动时重新注入的 `AGENTS.md`。

## Goals / Non-Goals

**Goals:**

- 一组对话一个文件，路径每次都能从会话号重新算出来。
- 会话号的来源在压缩之后仍然存在。
- 没有会话号时不把新对话粘到旧文件上。

**Non-Goals:**

- 不从 `prompts/` 目录里推断「最近的那个文件就是本对话」。
- 不保存助手回复。

## Decisions

### 文件名就是会话号

`prompts/<author>/<session-id>.md`

作者名仍取 `git config user.name` 的路径安全形式。会话号只保留字母、数字、点、下划线和连字符。

同一会话追加。日期不再进路径，因为压缩之后记不住第一句话是哪一天。

### 会话号按固定顺序取，每次重算

1. `OPENSPEC_SESSION_ID`
2. `CURSOR_AGENT_STORE_FILES_DIR`：取其中 `files` 的上一级目录名
3. `CLAUDE_SESSION_ID`，否则 `CLAUDE_CODE_SESSION_ID`
4. `CODEX_SESSION_ID`，否则 `CODEX_THREAD_ID`
5. `OPENCODE_SESSION_ID`
6. `HERMES_SESSION_ID`
7. `DSH_SESSION_ID`

这些值属于进程环境或 harness 的会话目录，不在被压缩的聊天记录里。启动说明每次都会重新注入，所以压缩之后仍然知道要按这个顺序重算。

没有命中时，新建 `prompts/<author>/unscoped-<YYYYMMDDHHMMSS>.md`。不读目录、不打开其他会话文件。这种 harness 若要保持一组对话不拆开，应自己设置 `OPENSPEC_SESSION_ID`。

备选是在作者目录放一个「当前会话」指针。新对话和压缩后的旧对话都没有聊天记忆，指针无法区分二者，会把无关对话 append 到一起。

### 文件内容

文首是 `author`、`session`、`started_at`。每一条用户 Prompt 用它的时间作标题，下面只放原文。

## Risks / Trade-offs

- [某个 harness 没有列出的变量名] → 用 `OPENSPEC_SESSION_ID` 补上，不新增猜测规则。
- [Cursor 的 store 目录以后改结构] → 规则写的是「`files` 的上一级目录名」，改结构时只改这一条。
