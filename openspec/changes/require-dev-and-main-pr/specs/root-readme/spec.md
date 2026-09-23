# Spec Delta

## ADDED Requirements

### Requirement: Development uses Dev-prefixed branches
根目录 `README.md` MUST 说明开发工作在 `Dev/*` 命名的 Git 分支上进行，并说明通过 Pull Request 将变更合入 `main`。

#### Scenario: Contributor starts repository work
- **WHEN** 贡献者查看根目录 `README.md` 中的开发规范
- **THEN** 文档明确要求使用 `Dev/*` 分支开展开发，并通过 Pull Request 合入 `main`

### Requirement: OpenSpec setup is documented across operating systems
根目录 `README.md` MUST 为 Windows、macOS 和 Linux 用户提供适用的 OpenSpec CLI 安装方式及仓库初始化步骤。说明 MUST 包含 Node.js 版本前提、项目根目录初始化命令，以及重启或重新加载工具集成的提示。

#### Scenario: Contributor configures OpenSpec on a supported operating system
- **WHEN** Windows、macOS 或 Linux 用户按 README 配置 OpenSpec
- **THEN** 用户能找到对应的 CLI 安装命令和共用的仓库初始化步骤
