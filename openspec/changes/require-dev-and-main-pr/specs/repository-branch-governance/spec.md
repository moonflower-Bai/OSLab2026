# Spec Delta

## Purpose

为仓库提供可审阅、可导入的 GitHub 分支规则配置，使 `main` 的更新统一经过 Pull Request，并能通过版本控制留存规则内容。

## ADDED Requirements

### Requirement: Main branch updates require pull requests
仓库 MUST 提供 GitHub repository ruleset JSON，目标仅为 `main` 分支，处于启用状态，并要求所有更新经 Pull Request。该规则集 MUST NOT 配置可绕过规则的角色。

#### Scenario: User imports the main protection ruleset
- **WHEN** 仓库管理员将随仓库提供的 JSON 作为 repository ruleset 导入
- **THEN** 启用的规则针对 `main`，并要求更新通过 Pull Request，且没有配置的 bypass actor

### Requirement: Ruleset file is portable and machine-readable
分支规则 JSON MUST 符合 GitHub repository ruleset 的可导入 JSON 结构，且 MUST 使用不绑定特定仓库所有者或名称的配置。

#### Scenario: Validate the ruleset configuration
- **WHEN** 工具解析规则文件
- **THEN** 文件是有效 JSON，包含 GitHub 导入所需的规则集字段，并以 `refs/heads/main` 选择目标分支
