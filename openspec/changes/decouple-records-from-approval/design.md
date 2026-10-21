# Design

## Context

动机见 proposal.md。规则通过三层生效：根规则 `AGENTS.md`（`CLAUDE.md` 仅导入它，Codex、DeepSeek harness 和 Hermes 直接读取它）、由 `scripts/sync_ai_workflows.py` 重放的平台技能和命令、以及 `scripts/check_governance.py` 的关键词检查。上一次简化在这几层都写入了“无 change 时不强制规划”。团队目前只使用 Claude Code、Codex、DeepSeek harness 和 Hermes，入口文件只有 `AGENTS.md` 和 `CLAUDE.md` 两类。`change-routing` 能力目前只存在于未归档的 `simplify-change-routing` delta 中。

## Goals / Non-Goals

**Goals:**
- 让实际使用的 harness 读到的第一条规则就是“先判断是否需要 change，再由授权决定是否暂停”。

**Non-Goals:**
- 不做 PR 级记录检查，规则须在动手前生效，事后补记录意义不大。
- 不维护 Cursor、OpenCode 适配；不删除其现有文件。

## Decisions

1. **两个独立判断写成表格放在 `AGENTS.md` OpenSpec 节最前面**。表格把“直接实现”放在“要不要暂停”一列而非“要不要建 change”一列。删除“不要因为……自动进入完整规划”“不强制先创建……”等否定开头句，直接处理的范围改为正向列举。备选方案是只加一句“授权不免除记录”，但旧段落仍会先被读到，冲突时 AI 倾向更宽松的一条，故不采用。
2. **授权下的执行顺序固定为：建/复用 change → 写产物与 Prompt 证据 → `openspec validate <change> --strict` → 实施 → 验证**。以严格校验作为可执行的“自检无误”标准。
3. **同步范围收窄为 `.agents`、`.claude`、`.hermes` 技能和 `.claude/commands/opsx` 命令**。`--check` 只检查这些路径，避免为不用的工具维护提示词。
4. **治理关键词检查**：`AGENTS.md` 新增必须出现的“先留记录”“严格校验”术语；把旧的“无 change 时，不强制先创建 proposal、delta spec、design、tasks 才能修改”加入废止列表，并在自检中加入对应回退样例。Cursor 规则文件的既有检查保持不变。

## Risks / Trade-offs

- [没有机械兜底，AI 仍可能忽略规则] → 规则开头即表格，平台提示词同步强调；后续如仍失效再考虑检查。
- [Cursor、OpenCode 文件保留旧措辞] → README 注明不再维护；需要时重新加入同步范围。
- [本 change 的 MODIFIED delta 依赖未归档的 `simplify-change-routing`] → 归档本 change 前先归档 `simplify-change-routing`；两者归档均需用户明确授权。

## Migration Plan

实施后运行 `python3 -B scripts/sync_ai_workflows.py --write` 重放提示词。Lab2 已有工作不补记录，Lab3 起按新规则执行。回滚即还原上述文件并再次 `--write`。
