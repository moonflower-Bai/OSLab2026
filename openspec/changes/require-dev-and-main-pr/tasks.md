# Tasks

## 1. 更新根目录开发和 OpenSpec 文档

- [x] 1.1 同步 `AGENTS.md`、`README.md`、`openspec/config.yaml` 和 Cursor 引导，明确实验 dev `Dev/LabX/UserName/branch` → stable `Dev/LabX/main` → release 根级 `LabX`，普通非实验 `Dev/*` → main；审阅角色及目标与 current/frozen 约束一致。
- [x] 1.2 补充 Windows、macOS、Linux 的 CLI 安装方式、Node.js 版本前提、仓库初始化/更新步骤和官方文档链接；核对每个平台都有安装路径且共同步骤可复制执行。npm 路径要求 Node.js 20.19.0 或更高版本。
- [x] 1.3 在共享提示词块增加根分支规范引用并同步 48 份技能与命令；运行 `python3 -B scripts/sync_ai_workflows.py --check` 并确认再次同步修改 0 份。

## 2. 提供并说明 main ruleset

- [x] 2.1 新增仅匹配 `refs/heads/main`、启用 `pull_request` 规则且没有 bypass actor 的 GitHub repository ruleset JSON；运行 `python3 -m json.tool .github/rulesets/main-requires-pull-request.json` 验证 JSON 语法。
- [x] 2.2 在 README 说明将 JSON 导入 GitHub repository ruleset 的入口和所需权限；外部前提是导入者具有该仓库规则编辑权限，实际远程导入不由本 change 执行。

## 3. 验证规格和最终改动

- [x] 3.1 新增分支格式、实验路径分类、章节匹配和 PR 目标检查及自检，CI 传入真实 head/base；用正反例矩阵和当前分支的定向治理检查验证，无需真实推送。
- [x] 3.2 运行 OpenSpec 当前 change、全量和归档严格校验、提示词检查及 `git diff --check`；确认 main ruleset、跨平台配置说明和实验实现未修改，并遵从本轮不推送、不提交、不归档的范围。

本轮验证记录（2026-10-03）：20 个分支正反例、8 个路径分类样例、实际 Dev/simplify-change-routing → main、当前 change Prompt、CI 参数及命令语法、章节范围和行尾检查通过。48 份提示词一致性通过，重复同步修改 0 份。OpenSpec 当前 change 校验通过，全量 6 项、归档 3 项通过，git diff --check 通过。未修改实验实现、main ruleset 或远程分支；没有新建 Git commit、推送或归档。
