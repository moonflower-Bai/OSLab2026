# Spec Delta

## ADDED Requirements

### Requirement: Development uses Dev-prefixed branches
根目录 `README.md` MUST 说明实验代码按 dev `Dev/LabX/UserName/branch` → stable `Dev/LabX/main` → release 根级 `LabX` 流转，普通非实验开发使用 `Dev/*` 并以 main 为 PR 目标。文档 MUST 说明 X 对应实验章节，UserName 是开发者标识。

#### Scenario: Contributor starts repository work
- **WHEN** 贡献者查看根目录 `README.md` 中的开发规范
- **THEN** 文档能区分实验 dev、stable、release 和普通非实验开发分支，并说明实验成果最终提交到根级对应 `LabX`

### Requirement: OpenSpec setup is documented across operating systems
根目录 `README.md` MUST 为 Windows、macOS 和 Linux 用户提供适用的 OpenSpec CLI 安装方式及仓库初始化步骤。说明 MUST 包含 Node.js 版本前提、项目根目录初始化命令，以及重启或重新加载工具集成的提示。

#### Scenario: Contributor configures OpenSpec on a supported operating system
- **WHEN** Windows、macOS 或 Linux 用户按 README 配置 OpenSpec
- **THEN** 用户能找到对应的 CLI 安装命令和共用的仓库初始化步骤
