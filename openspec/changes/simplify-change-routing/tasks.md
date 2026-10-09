# Tasks

## 1. 实施分流规则

- [x] 1.1 更新 `AGENTS.md`，先识别明确实施批准及快速、直接修改请求，再判断默认直接处理或完整流程，并加入持续授权、仅确认未批准增量、撤回时停止及可一次授权多操作的规则；逐项审阅本 change 场景，确认维护不重复审批、明确直接实施不再等待新消息，章节保护及提交授权保留。
- [x] 1.2 同步 `openspec/config.yaml`、`README.md` 和 `.cursor/rules/openspec.mdc` 的分流及持续授权语义；用 `rg -n '直接处理|完整|授权|批准|Propose|Apply|Archive|Commit' AGENTS.md README.md openspec/config.yaml .cursor/rules/openspec.mdc` 及人工比对确认没有恢复全量规划或重复审批要求，Claude 原生导入入口仍有效。
- [x] 1.3 同步根规则和 `prompts/README.md` 的无 change 不保存策略，并在规则中明确新分流优先于历史一刀切条款；审阅确认不新建会话档案、不读取其他 change 的 Prompt、不改写旧 change 的记录。
- [x] 1.4 同步五类技能和三类命令目录的 48 份提示词，修改正文中的重复审批条款，新增可重放的 `scripts/sync_ai_workflows.py`；运行 `python3 -B scripts/sync_ai_workflows.py --check` 并验证 `--write` 重复运行不产生差异，检查 frontmatter 和平台命令拼写保留。

## 2. 验证与交付

- [x] 2.1 扩展 `scripts/check_governance.py` 的规则检查和自检，覆盖合法直接维护、明确快速授权、持续授权声明、旧一刀切规则及提示词回退；通过模块导入调用 `self_test()` 和规则检查纯函数，确认正确规则通过、回退样例失败，不运行会读取其他 change Prompt 的全量扫描。
- [x] 2.2 运行 `openspec validate simplify-change-routing --strict`、`openspec validate --all --strict`、`openspec validate --archived --strict` 和 `git diff --check`；仅对本 change 的 prompt.md 调用现有证据检查纯函数，审阅最终 diff 仅含计划内治理文件和本 change，报告结果，只有尚无归档或提交授权时才等待对应授权。

外部前提为可用的 OpenSpec CLI、Python 3 和 Git；缺失时明确报告。本 change 不修改章节代码或具体 LaTeX 文档，不需要 RISC-V 工具链或 LaTeX 编译器。

验证记录（2026-10-03）：48 份默认模板回放与回退检测、幂等性、frontmatter 和全部命令代码块保留检查通过；未知模板及重复引导被拒绝；重复同步修改 0 份。治理自检、实际规则、当前 change 的 Prompt、章节范围及行尾检查通过。OpenSpec 当前 change 校验通过，全量 6 项、归档 3 项通过，`git diff --check` 通过。未读取其他 change 的 Prompt，未修改章节文件，未归档或创建 Git commit。
