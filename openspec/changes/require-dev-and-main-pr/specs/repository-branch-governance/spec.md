# Spec Delta

## Purpose

为仓库定义可验证的实验开发、非实验维护和章节最终提交分支规则，使实验成果集中到根级 LabX，同时保留 main 的 PR 保护配置。各工具和本地及 CI 检查按同一分支规范执行，并区分命名检查与实际远程保护。

## ADDED Requirements

### Requirement: Development branches distinguish experiment code
含当前章节实验代码的修改 MUST 使用 dev 四段 `Dev/LabX/UserName/branch` 或 stable 三段 `Dev/LabX/main` 分支，LabX MUST 与 `lab-status.json.current` 对应，UserName 和 branch MUST 非空。无实验代码的文档、LaTeX 或治理维护 MUST 允许非空 `Dev/*`，MUST NOT 被强制要求开发者层级；已有合法实验 dev/stable 分支的相关文档 MUST 按该角色流转。混合修改 MUST 按实验代码要求判断；冻结章节限制 MUST 保留。

#### Scenario: Current lab code is changed
- **WHEN** 当前章节为 lab1 且修改实验代码
- **THEN** dev `Dev/Lab1/UserName/fix-boot` 与 stable `Dev/Lab1/main` 符合各自格式，普通 Dev/fix-boot、缺少层级或使用其他章节均被拒绝

#### Scenario: A document in the lab directory is changed
- **WHEN** diff 只有实验目录内的 Markdown 或 LaTeX 文档
- **THEN** 普通 `Dev/docs` 分支通过非实验分支要求，不必使用四段层级

### Requirement: Lab deliverables use root-level submission branches
实验分支 MUST 按 dev → stable → release 流转：dev PR 目标 MUST 为 `Dev/LabX/main`，stable PR 目标 MUST 为根级 release `LabX`，如 `Lab1`。MUST NOT 使用嵌套 `main/LabX` 或开发分支充当最终提交分支，MUST NOT 默认从 dev 跳过 stable。普通非实验 PR MUST 沿用 `main`。AI MUST NOT 将规范更新理解为创建、重命名或推送实际分支的授权。

#### Scenario: A lab contribution is ready for review
- **WHEN** lab1 实验代码从符合格式的开发分支发起 PR
- **THEN** dev 先进入 `Dev/Lab1/main`，stable 再进入根级 `Lab1`，不进入其他章节或 main

#### Scenario: Dev attempts to skip stable
- **WHEN** dev `Dev/Lab1/UserName/fix-boot` 直接以 release `Lab1` 为 PR 目标
- **THEN** 检查拒绝该目标，指出应先进入 stable `Dev/Lab1/main`

#### Scenario: A governance contribution is ready for review
- **WHEN** 仅修改仓库治理和文档
- **THEN** 源分支为 Dev/*，PR 目标为 main

### Requirement: Local and CI checks share branch rules
本地与 CI MUST 共用修改内容、源分支和 PR 目标检查；本地未指定 PR 目标时 MUST 只检查开发分支。PR CI MUST 使用实际 head/base 分支而非合并 checkout 的 detached HEAD 名称。各 AI 工具提示词 MUST 引用根规则的同一分支标准，MUST NOT 自行使用相反前缀。

#### Scenario: Pull request CI checks a merge checkout
- **WHEN** GitHub Actions 在 detached HEAD 验证实验 PR
- **THEN** 检查使用实际 dev/stable head 与对应 stable/release base，并指出非法格式或错误目标

#### Scenario: Local maintenance is checked
- **WHEN** 在普通 Dev/* 分支验证无实验代码的维护，未指定 PR 目标
- **THEN** 检查通过，且不要求访问或修改远程仓库

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
