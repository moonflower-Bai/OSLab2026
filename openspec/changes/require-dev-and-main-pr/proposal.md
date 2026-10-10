# Proposal

## Why

仓库需要区分实验代码、非实验维护和最终实验提交的分支，避免所有工作都只使用笼统的 `Dev/*` 并统一合入 main。保留已有 OpenSpec 跨平台说明和 main PR 配置，同时让实验成果集中到根级 `LabX` 分支。

## What Changes

- 实验代码开发分支使用 `Dev/LabX/UserName/branch`，章节与 `lab-status.json.current` 对应；非实验修改使用 `Dev/*`。
- 实验按 dev → stable → release 流转：dev 为 `Dev/LabX/UserName/branch`，stable 为 `Dev/LabX/main`，release 为根级 `LabX`，如 `Lab1`；实验 PR 分别进入对应 stable、release，普通非实验 PR 保留以 `main` 为目标。
- 同步根规则、README、OpenSpec 配置和各 AI 工具提示词；新增本地与 CI 分支格式及 PR 目标检查。
- 在根 README 增加 Windows、macOS、Linux 的 OpenSpec CLI 安装说明，以及在仓库根目录初始化工具集成的共同步骤。
- 新增可导入的 GitHub repository ruleset JSON，针对 `main` 启用 PR 更新要求且不配置绕过角色。
- 扩展 README 与仓库分支治理的 OpenSpec 规格。

## Capabilities

### New Capabilities

- `repository-branch-governance`：规定分支分流、章节成果分支、本地及 CI 校验，并保留 main 分支保护配置。

### Modified Capabilities

- `root-readme`：区分实验与非实验分支、说明根级 `LabX` 提交分支，并保留跨平台 OpenSpec 初始化说明。

## Impact

- `AGENTS.md`、`README.md`、`openspec/config.yaml`、Cursor 入口和各平台技能及命令：统一分支要求。
- `scripts/check_governance.py`、`scripts/sync_ai_workflows.py`、`.github/workflows/governance.yml`：本地、CI 及提示词校验。
- `.github/rulesets/main-requires-pull-request.json`：可导入的 main 分支规则集。
- `openspec/specs/root-readme/spec.md` 及新增的 `repository-branch-governance` 规格。
- GitHub repository rulesets：应用 JSON 文件仍由具有仓库规则编辑权限的人员导入；本变更不直接访问或修改远程仓库设置。

## Non-goals

- 不迁移章节目录或修改实验实现，不自动创建、重命名或推送任何 `LabX` 分支。
- 不自动在远程 GitHub 仓库安装 ruleset。
- 不增加必需审查人数、状态检查或合并方式限制。
- GitHub ruleset 仍只保护 `main`；新增本地与 CI 分支检查不等于已部署远程 `LabX` 保护规则。用户本轮取消推送，不进行推送、提交或归档。
