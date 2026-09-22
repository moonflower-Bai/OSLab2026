# 仓库指令

这些规则在会话开始时自动生效，是本仓库 AI 辅助开发的唯一根级规则入口。各工具应通过自己的原生入口读取本文件，不要复制出相互独立的规则。

## OpenSpec 工作流

用户要求修改仓库、实现实验或生成内核代码时，必须先留下 OpenSpec 记录。只回答问题且不修改文件时，不创建 change。

一次变更分为四个独立授权阶段：

1. **Propose**：读取 `openspec/config.yaml` 和已有主规格；不存在对应 change 时，使用当前平台提供的 OpenSpec propose workflow 创建 proposal、delta spec、design 和 tasks，然后停止，等待用户明确授权实现。
2. **Apply**：仅在用户明确要求应用该 change 后，使用 OpenSpec apply workflow 按 tasks 实现和验证。实现授权不包含归档。
3. **Archive**：仅在用户明确同意归档后，使用 OpenSpec archive workflow 合并规格并归档 change。
4. **Commit**：仅在用户明确要求提交后创建 Git commit。提案、实现或归档授权均不自动包含提交。

使用平台原生的 OpenSpec workflow；不要依赖某个工具私有的技能安装路径。规格、相关 Prompt 证据和实现应一起进入最终提交，聊天记录本身不算项目留痕。

## 修改范围

`lab-status.json` 是章节状态的唯一事实来源：

- 实验实现只能修改 `current` 指定的章节目录。
- `frozen` 中的章节不得修改。
- OpenSpec 规格、change、Prompt 证据、治理规则、CI 和仓库级文档属于工作流元数据，不受“只改当前章节目录”的限制。
- 章节目录内更具体的 `AGENTS.md` 可以补充构建和验证要求，但不能放宽根规则。

## 保存用户 Prompt

Prompt 证据只保存在当前 change 的 `prompt.md`，细节见 `prompts/README.md`。

- 只有直接决定当前 change 的需求、范围、设计取舍、验收标准或授权的用户原文才可保存。
- 普通问答、状态询问、与实现无关的对话不保存。
- 只要一条消息含姓名、学号、身份证号、凭证或其他敏感值，整条原文都不得保存，也不得保存脱敏、删节或改写后的副本；只可写不含原文内容的省略记录。
- 保存前必须人工确认相关性和敏感性；自动扫描只作为辅助门禁。
- 不要列举、搜索、读取或迁移其他 change 的 Prompt 证据或遗留会话 Prompt 文件。
