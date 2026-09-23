# Proposal

## Why

仓库需要统一开发分支和 main 合并方式，避免把开发提交直接推到 main。根 README 也缺少跨 Windows、macOS 和 Linux 的 OpenSpec 配置说明，导致新环境无法按仓库流程开始工作。

## What Changes

- 更新根 README，要求在 `Dev/*` 分支开展开发，并说明通过 Pull Request 将变更合入 `main`。
- 在根 README 增加 Windows、macOS、Linux 的 OpenSpec CLI 安装说明，以及在仓库根目录初始化工具集成的共同步骤。
- 新增可导入的 GitHub repository ruleset JSON，针对 `main` 启用 PR 更新要求且不配置绕过角色。
- 扩展 README 与仓库分支治理的 OpenSpec 规格。

## Capabilities

### New Capabilities

- `repository-branch-governance`：规定 main 分支保护配置的机器可读交付内容及 PR 要求。

### Modified Capabilities

- `root-readme`：增加 `Dev/*` 开发分支约定和跨平台 OpenSpec CLI／项目初始化说明。

## Impact

- `README.md`：开发分支约定、OpenSpec 安装与初始化文档。
- `.github/rulesets/main-requires-pull-request.json`：可导入的 main 分支规则集。
- `openspec/specs/root-readme/spec.md` 及新增的 `repository-branch-governance` 规格。
- GitHub repository rulesets：应用 JSON 文件仍由具有仓库规则编辑权限的人员导入；本变更不直接访问或修改远程仓库设置。

## Non-goals

- 不迁移章节目录或修改实验实现。
- 不自动在远程 GitHub 仓库安装 ruleset。
- 不增加必需审查人数、状态检查或合并方式限制。
- GitHub ruleset 只保护 `main` 的更新路径；`Dev/*` 分支命名作为 README 中的开发约定记录。
