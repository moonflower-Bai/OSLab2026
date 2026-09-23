# Tasks

## 1. 更新根目录开发和 OpenSpec 文档

- [x] 1.1 在 `README.md` 写明开发分支使用 `Dev/*` 前缀、经 Pull Request 合入 `main`，并保留现有章节范围规则；用 `git diff --check` 检查文档 diff。
- [x] 1.2 补充 Windows、macOS、Linux 的 CLI 安装方式、Node.js 版本前提、仓库初始化/更新步骤和官方文档链接；核对每个平台都有安装路径且共同步骤可复制执行。npm 路径要求 Node.js 20.19.0 或更高版本。

## 2. 提供并说明 main ruleset

- [x] 2.1 新增仅匹配 `refs/heads/main`、启用 `pull_request` 规则且没有 bypass actor 的 GitHub repository ruleset JSON；运行 `python3 -m json.tool .github/rulesets/main-requires-pull-request.json` 验证 JSON 语法。
- [x] 2.2 在 README 说明将 JSON 导入 GitHub repository ruleset 的入口和所需权限；外部前提是导入者具有该仓库规则编辑权限，实际远程导入不由本 change 执行。

## 3. 验证规格和最终改动

- [x] 3.1 运行 `openspec validate "require-dev-and-main-pr" --type change --strict` 验证 delta 规格；命令必须成功。
- [x] 3.2 运行 `git diff --check` 并复核 README、ruleset JSON 与需求一致；确认没有修改实验实现目录。
