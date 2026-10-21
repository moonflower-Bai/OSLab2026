# Tasks

## 1. 规则入口

- [x] 1.1 重写 `AGENTS.md` 的 OpenSpec 工作流节：开头用表格分开“是否需要 change（按影响）”与“是否暂停（按授权）”，写明授权下“产物 → 严格校验 → 实施”的顺序及不确定时创建 change；删除劝阻使用 OpenSpec 的否定句，保留持续授权、操作授权边界、分支与 Prompt 规则。逐条对照本 change 的 spec 场景人工审阅。
- [x] 1.2 同步 `openspec/config.yaml`、`README.md`（含 harness 范围说明），必要时同步 `prompts/README.md`；用 `rg -n '不强制' AGENTS.md README.md openspec/config.yaml prompts/README.md` 确认不再存在“授权即可跳过 change”的表述。

## 2. 平台提示词

- [x] 2.1 修改 `scripts/sync_ai_workflows.py`：同步范围收窄为 `.agents`、`.claude`、`.hermes` 技能与 `.claude/commands/opsx`；`POLICY_BLOCK` 与 propose Planning boundary 等替换文本表达“授权取消暂停、不取消记录；先完成并严格校验产物再实施”。运行 `--write` 后 `--check` 通过，第二次 `--write` 不产生差异，确认 `.cursor`、`.opencode` 无改动。

## 3. 治理检查

- [x] 3.1 更新 `scripts/check_governance.py` 的 `rule_violations` 必需术语与废止措辞，并在 `self_test()` 加入“授权即可跳过 change”回退样例；运行 `python3 -B scripts/check_governance.py --self-test` 通过。

## 4. 验证与交付

- [x] 4.1 运行 `openspec validate decouple-records-from-approval --strict`、`openspec validate --all --strict`、`git diff --check`；审阅 diff 只含计划内治理文件与本 change，不含章节代码。
- [x] 4.2 在 `Dev/*` 分支提交，推送并用 `gh` 创建以 `main` 为目标的 PR，CI 通过后合并。

## 5. 状态机重述（追加）

- [x] 5.1 将 `AGENTS.md` OpenSpec 节改写为状态机（TRIAGE、ANSWER、DIRECT、RECORD、GATE、WAIT、APPLY、DONE、STOPPED）、封闭的直接处理白名单和跨状态约束（写入范围、章节范围、Archive/Commit）；对照 spec 中“黑盒测试”“写入范围”场景人工审阅。
- [x] 5.2 同步 `openspec/config.yaml`、`README.md` 与 `sync_ai_workflows.py` 的 `POLICY_BLOCK`；运行 `--write`、`--check`，第二次 `--write` 无差异。
- [x] 5.3 更新 `check_governance.py` 必需术语（状态机、白名单、写入范围）与自检；运行 `python3 -B scripts/check_governance.py --self-test` 通过，`openspec validate --all --strict` 通过。
- [x] 5.4 另一会话结束占用工作树后，把改动迁移到 `Dev/openspec-record-first`；迁移与提交需用户另行授权提交时执行。

外部前提：OpenSpec CLI、Python 3、Git、已登录的 `gh`。不归档本 change。

验证记录（2026-10-21）：`sync_ai_workflows.py --write` 修改 24 / 24 份，第二次 0 份，`--check` 通过 24 份，`.cursor`、`.opencode` 无改动。`check_governance.py --self-test` 通过（含“授权跳过 change”回退样例）。`openspec validate decouple-records-from-approval --strict` 通过（INFO：归档前需先归档 `simplify-change-routing`），`openspec validate --all --strict` 7 项通过，`git diff --check` 通过。未修改章节代码。
