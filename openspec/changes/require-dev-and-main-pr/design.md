# Design

## Context

当前 README 已介绍章节范围、Prompt 和 OpenSpec 流程，但没有规定 Git 分支前缀，也没有说明不同操作系统如何安装 CLI。仓库有本地 OpenSpec 根目录和 `root-readme` 主规格。ruleset 应保持仓库无关，导入后只匹配 `main`。

## Goals / Non-Goals

**Goals:**

- 让贡献者能从 README 了解 `Dev/*` 开发分支约定和 OpenSpec 跨平台安装、初始化流程。
- 提供可由 GitHub repository ruleset 导入的配置，要求 `main` 更新经 Pull Request。

**Non-Goals:**

- 不通过规则强制每个工作分支都匹配 `Dev/*`；该分支前缀是 README 中的开发约定。
- 不在本地或远程 GitHub 仓库自动应用规则集。
- 不增加审查数、状态检查、代码所有者审查或其他合并门槛。

## Decisions

### README 文档按操作系统区分 CLI 安装，初始化步骤保持共用

Windows 使用 Node.js 安装器提供的 npm，全局安装 OpenSpec；macOS 可用 Homebrew 或 npm；Linux 可用 npm，也可使用 Homebrew/Nix。三者都先验证 `openspec --version`，然后在仓库根目录按所用 AI 编程工具运行 OpenSpec 初始化/更新步骤。这样把操作系统相关的 CLI 安装与项目内工具集成分开，避免重复介绍规格工作流。

备选是只记录 npm 单一命令。它虽然跨平台，但没有说明 Node 前提以及已有 Homebrew/Nix 环境的安装路径。

### 使用 repository ruleset API 结构作为可移植配置

JSON 使用 `target: branch`、`enforcement: active`、`conditions.ref_name.include: ["refs/heads/main"]`，并设置 GitHub 的 `pull_request` 规则。`bypass_actors` 为空数组，不为任何用户、团队或应用配置绕过权限；PR 规则只要求 PR，不额外要求审批或状态检查。配置不包含仓库 owner/name 或实例 URL，便于导入不同仓库。

备选是仅依赖 README 约定或传统 branch protection UI。前者不能由 GitHub 执行，后者没有可版本控制、可导入的单文件配置。

### GitHub 部署由仓库管理员显式执行

提交规则文件和导入说明，但不使用 API 或凭证修改远程仓库。导入需要拥有仓库规则编辑权限的人员在目标仓库确认应用结果。

备选是自动化调用 GitHub API。当前没有用户授权的远程目标、凭证或部署要求，自动应用会越过用户要求的交付范围。

## Risks / Trade-offs

- [Ruleset 只能约束 GitHub 上 `main` 的更新，无法强制本地工作分支命名] → README 清楚区分 Git 分支约定和 GitHub 服务端保护范围。
- [GitHub 导入界面或格式可能变更] → 用官方 repository rulesets 的结构生成 JSON，并在实现时解析 JSON、运行 OpenSpec 严格校验；README 链接至官方导入说明。
- [管理员仍可修改或删除规则集] → 文件纳入版本控制，README 说明导入需要有仓库规则编辑权限的人员完成。

## Migration Plan

1. 增补根 README 的开发分支及 OpenSpec 安装/初始化说明。
2. 添加 main PR ruleset JSON 和从 GitHub 设置导入的短步骤。
3. 解析 JSON、检查 diff 和 OpenSpec 规格；不触碰远程仓库设置。
4. 用户在目标仓库导入后，可按需在 GitHub ruleset 页面确认规则已启用。若要回滚，删除或禁用该 ruleset 文件及其远程导入配置。
