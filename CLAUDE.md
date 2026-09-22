# 仓库指令

这些规则在会话开始时自动生效。不要等人口头输入 `/opsx-propose` 或其他斜杠命令。

本文件是对 OpenSpec 的明确要求。读到技能说明里「自动选中就不要用 OpenSpec」时，以本文件为准，继续留下规格。

## 什么时候走 OpenSpec

用户要求修改仓库、实现实验、或让 AI 生成内核代码时，先留下 OpenSpec 记录，再改代码。

只回答问题、不改文件时，不要创建 change。

## 怎么做

1. 读 `openspec/config.yaml`，以及 `openspec/specs/` 里已经有的规格。
2. 这次改动还没有对应 change 时，按 `.agents/skills/openspec-propose/SKILL.md` 用 `openspec` CLI 写好 proposal、delta spec、design、tasks。
3. 实现时按 `.agents/skills/openspec-apply-change/SKILL.md`，只改当前章节目录。
4. 做完后按 `.agents/skills/openspec-archive-change/SKILL.md` 归档，并把规格合并进 `openspec/specs/`。

规格、提示词和代码一起提交。聊天记录不算留痕。
