# Design

## Context

当前 README、跨平台初始化说明和 main PR ruleset 已实现，但分支约定只区分 `Dev/*` 和 main。现需按实验代码、非实验维护、章节最终成果分流；当前章节为 `lab1`，开发分支为非实验用途的 `Dev/simplify-change-routing`，可以继续使用。已有共享提示词同步入口可把根规则引用同步到 48 份技能和命令。

## Goals / Non-Goals

**Goals:**

- 让贡献者和各 AI 工具一致区分 dev `Dev/LabX/UserName/branch`、stable `Dev/LabX/main`、release `LabX` 及普通非实验 `Dev/*`。
- 本地按当前分支检查，PR CI 使用真实源分支和目标分支而非 detached merge checkout 名称检查。
- 提供可由 GitHub repository ruleset 导入的配置，要求 `main` 更新经 Pull Request。

**Non-Goals:**

- 不把最终 `LabX` 当成直接开发分支，不更改实际远程分支或其保护设置。
- 不在本地或远程 GitHub 仓库自动应用规则集。
- 不增加审查数、状态检查、代码所有者审查或其他合并门槛。

## Decisions

### 以当前章节和修改内容判断分支

实验代码位于 `labN/`，包含内核、库、构建脚本和链接脚本。该目录内 Markdown、LaTeX、PDF、图片、`AGENTS.md` 等文档素材属于非实验代码；其他仓库治理和配置也属于非实验修改。实验 dev 分支要求四段 `Dev/LabN/UserName/branch`，stable 分支要求三段 `Dev/LabN/main`，`LabN` 与当前章节对应；`UserName` 是开发者标识，规范不要求收集真实姓名。无实验代码的普通维护只要求非空 `Dev/*`；已使用合法实验 dev/stable 分支的相关文档仍按其角色流转。

源开发分支与稳定、最终提交分支分开：dev PR 目标为 `Dev/LabN/main`，stable PR 目标为根级 `LabN`，普通非实验 PR 目标沿用 main。本地未指定 PR 目标时只检查源分支；不根据 release 名称推断冻结状态，不放宽 current/frozen 限制，不直接在 release 开发。

### 本地与 CI 共用分支检查

在现有治理脚本中新增纯函数检查和自检；CLI 接受 `--branch`、`--target`，本地默认读取当前符号分支。CI 从 `github.head_ref` 和 `github.base_ref` 通过环境变量传入，避免使用 checkout 的 detached HEAD。非法层级、章节不一致、dev 跳过 stable、stable 进入错误 release、普通非实验 PR 目标 LabN 等情况返回非零。

根规则存放完整标准，README 给出示例，配置和各工具引导引用该标准；共用提示词块增加分支规则引用并同步 48 份。保留平台命令语法和此前的持续授权规则。

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

- [本地或 CI 分支检查不是远程保护] → README 区分命名检查与仅匹配 main 的 ruleset，不宣称 LabN 已受 GitHub 保护。
- [PR checkout 处于 detached HEAD] → CI 显式传入源、目标分支，本地缺少开发分支时明确报错。
- [实验目录文档误判为代码] → 单独识别文档素材，混合代码和文档的 diff 按实验代码处理。
- [GitHub 导入界面或格式可能变更] → 用官方 repository rulesets 的结构生成 JSON，并在实现时解析 JSON、运行 OpenSpec 严格校验；README 链接至官方导入说明。
- [管理员仍可修改或删除规则集] → 文件纳入版本控制，README 说明导入需要有仓库规则编辑权限的人员完成。

## Migration Plan

1. 更新根规则、README、配置、平台引导与本 change 的 delta spec。
2. 增加分支检查、自检和 PR CI 输入，同步 48 份提示词。
3. 执行分支矩阵、实际当前分支、提示词和 OpenSpec 严格校验；只检查当前 change 的 Prompt。
4. 保留旧 main ruleset，本轮不推送、提交或归档；回滚本轮元数据修改不影响实验实现和远程设置。
