# Tasks

## 1. 统一规则入口和授权边界

- [x] 1.1 重写 `AGENTS.md`，明确 propose、apply、archive、commit 的分阶段授权、实验实现与工作流元数据的范围差异，并确认文件不再硬编码 `.agents/skills/` 路径。
- [x] 1.2 把 `CLAUDE.md` 改成导入 `AGENTS.md` 的最小入口，并精简 Cursor always-apply rule；分别检查入口仍能指向根规则和平台原生 OpenSpec workflow。
- [x] 1.3 同步 `openspec/config.yaml` 中的章节、授权和术语约束，并确认它与 `AGENTS.md` 没有相反要求。

## 2. 定义章节状态和验证方式

- [x] 2.1 新增 `lab-status.json`，把 `lab1` 标为当前章节、冻结列表设为空，并用解析检查确认字段和值有效。
- [x] 2.2 新增 `lab1/AGENTS.md`，记录工具链前提、确定会终止的构建命令、QEMU 运行边界和成功输出，并用 `make -C lab1` 验证基础构建。（已执行；本机缺少 `riscv64-unknown-elf-gcc`，按外部工具链条件记录。）
- [x] 2.3 更新根 `README.md`，从 `lab-status.json` 描述实际进度、链接章节局部说明，并确保只陈述仓库中真实存在的自动检查。

## 3. 建立自动治理门禁

- [x] 3.1 新增 `.gitattributes` 固定文本为 LF，并规范化本 change 触及的治理文件；用 `git diff --check` 确认没有行尾错误。
- [x] 3.2 新增本地治理检查入口，验证 OpenSpec 严格校验、`lab-status.json`、冻结章节 diff、规则入口和 LF 行尾，并用故障输入或脚本自检证明每类失败会返回非零状态。
- [x] 3.3 新增 GitHub Actions workflow 调用同一个治理检查入口，并检查 workflow 的基线参数能够覆盖 pull request 和直接 push。

## 4. 补齐规格并完成集成验证

- [x] 4.1 修正 `root-readme` 主规格的 Purpose，并在实现完成后运行 `openspec validate --all --strict` 和 `openspec validate --archived --strict`，确认全部通过。
- [x] 4.2 运行本地治理检查和 lab1 构建，记录无法运行的外部工具链条件，并确认没有修改内核实现文件。
- [x] 4.3 审查最终 diff，确认 Prompt 保存策略已按 `restrict-prompt-archive` 切换，并向用户报告实现结果后等待 archive 授权。
