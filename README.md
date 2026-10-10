# OSLab2026

## 指导书

[riscv64-ucore 操作系统实验指导书](http://8.135.34.58/lab2026/_book/)

- [lab0 预备起](http://8.135.34.58/lab2026/_book/lab0/intro.html)
  - 环境、工具链、QEMU
- [lab0.5 AI 驱动的操作系统实验](http://8.135.34.58/lab2026/_book/lab0.5/intro.html)
  - 提示词规格与 AI 协作方式
- [lab1 最小可执行内核](http://8.135.34.58/lab2026/_book/lab1/lab1.html)
- lab2 至 lab9
  - [lab2 物理内存和页表](http://8.135.34.58/lab2026/_book/lab2/lab2.html)
  - [lab3 中断](http://8.135.34.58/lab2026/_book/lab3/lab3.html)
  - [lab4 进程管理](http://8.135.34.58/lab2026/_book/lab4/lab4.html)
  - [lab5 用户程序](http://8.135.34.58/lab2026/_book/lab5/lab5.html)
  - [lab6 进程调度](http://8.135.34.58/lab2026/_book/lab6/lab6.html)
  - [lab7 同步互斥](http://8.135.34.58/lab2026/_book/lab7/lab7.html)
  - [lab8 文件系统](http://8.135.34.58/lab2026/_book/lab8/lab8.html)
  - [lab9 页面置换与内存映射](http://8.135.34.58/lab2026/_book/lab9/lab9.html)
- [附录](http://8.135.34.58/lab2026/_book/appendix/intro.html)

## 开发进度

`lab-status.json` 是章节状态的唯一事实来源。当前章节为 `lab1`，冻结章节为空。

- 已放入仓库
  - `lab1/`：最小内核骨架，当前练习是阅读启动流程；构建与运行要求见 [lab1/AGENTS.md](lab1/AGENTS.md)
  - [Lab1 练习 1 与 2 的截图说明和实验记录](lab1/report/record_Lab1/README.md)：原始截图、GDB 日志、逐图核对结果和报告整理依据
  - `0_environment_setup.md`、`3_startdash.md`：环境说明
- 尚未放入
  - `lab2/` 至 `lab9/`

## 开发规范

- 章节状态
  - 一章一个目录：`lab1/`、`lab2/`、……
  - 只修改 `lab-status.json` 指定的当前章节；冻结章节不可修改
  - 单章命令和工具链要求写在该章的 `AGENTS.md`
- 开发分支
  - 实验按 `dev → stable → release` 流转，`LabX` 对应章节编号，`UserName` 是开发者标识
  - dev：`Dev/LabX/UserName/branch`，例如 `Dev/Lab1/UserName/fix-boot`，PR 目标为对应 stable
  - stable：`Dev/LabX/main`，例如 `Dev/Lab1/main`，PR 目标为对应 release
  - release：根级 `LabX`，例如 `Lab1`，用于最终实验提交；不直接开发或默认跳过 stable
  - 非实验代码、文档或治理维护使用普通 `Dev/*`，例如 `Dev/docs`，PR 目标仍为根级 `main`
  - 实验分支的章节须与 `lab-status.json.current` 对应；实验目录内纯 Markdown、LaTeX、图片等修改也可使用普通 `Dev/*`，混合代码修改按实验要求检查
  - 可导入的 GitHub `main` 保护规则见 [main-requires-pull-request.json](.github/rulesets/main-requires-pull-request.json)
- OpenSpec
  - 明确批准实施或要求快速、直接修改时，直接完成指定范围；局部 bug 修复、文档和 LaTeX 编辑默认直接处理，不强制创建 change
  - 尚未获准直接实施的新实验功能、规格行为变化、较大重构和治理策略调整先创建 change；只对尚未批准的新需求或实质范围扩展请求确认
  - 一次实施授权覆盖范围内的实现、修复、验证重试和规划同步，不逐任务或逐轮暂停；归档和提交需明确授权，可在同一条指令中一起给出
  - 根规则见 [AGENTS.md](AGENTS.md)，OpenSpec 配置见 [openspec/config.yaml](openspec/config.yaml)
  - 本地与 GitHub Actions 共用 `scripts/check_governance.py` 检查分支、规格、范围、规则和 Prompt 门禁；PR CI 显式传入 head/base，避免使用 detached HEAD 名称
- 用户 Prompt
  - 只保存直接影响 change 且不含敏感值的用户原文
  - 没有 change 的直接维护不保存 Prompt，也不为保存 Prompt 创建 change
  - 证据放在对应 change 的 `prompt.md`，不提交按会话生成的归档
  - 细节见 [prompts/README.md](prompts/README.md)

## 配置 OpenSpec

OpenSpec CLI 安装在开发者自己的机器上；项目规格和 change 则保存在本仓库。npm 安装方式需要 Node.js 20.19.0 或更高版本。安装方式详见 [OpenSpec 官方安装文档](https://openspec.dev/docs/installation)。

### Windows

1. 从 [nodejs.org](https://nodejs.org/) 安装 Node.js 20.19.0 或更高版本。
2. 在 PowerShell 或 Windows Terminal 中运行：

   ```powershell
   npm install -g @fission-ai/openspec@latest
   ```

### macOS

安装了 Homebrew 的用户可运行：

```sh
brew install openspec
```

也可安装 Node.js 20.19.0 或更高版本后运行：

```sh
npm install -g @fission-ai/openspec@latest
```

### Linux

使用 Node.js 20.19.0 或更高版本后，运行：

```sh
npm install -g @fission-ai/openspec@latest
```

也可通过 Homebrew 安装，或使用 Nix：

```sh
brew install openspec
# 或
nix profile install github:Fission-AI/OpenSpec
```

### 在仓库中启用工具集成

安装后先检查 CLI：

```sh
openspec --version
```

在项目根目录运行 `openspec init`，并在交互界面选择正在使用的 AI 编程工具。也可用工具 ID 非交互配置，例如 `openspec init --tools codex,claude`；可用 ID 以 `openspec init --help` 为准。本仓库已包含 OpenSpec 配置，可直接使用其已有 change 和规格。

仓库对 Codex、Claude、Cursor、OpenCode 和 Hermes 的 OpenSpec 技能及命令提示词做了本地适配，统一遵从根规则的分支规范、分流和持续授权逻辑。新增工具或执行 `openspec update` 后，在重新加载 IDE/Agent 前恢复并检查本地适配：

```sh
python3 -B scripts/sync_ai_workflows.py --write
python3 -B scripts/sync_ai_workflows.py --check
```

同步脚本保留各平台的命令语法，治理门禁会检测提示词缺失或审批规则回退。CLI 更新导致未知模板时，先按提示审阅适配规则，再重新加载工具。

### 导入 main 分支保护规则

具有仓库管理员权限或 `edit repository rules` 权限的人员可将上述 JSON 导入 GitHub：打开仓库 **Settings → Rules → Rulesets**，选择 **New ruleset → Import a ruleset**，选中 JSON 文件，检查内容后点击 **Create**。导入后确认 ruleset 处于 Active 状态且目标是 `main`。详细步骤见 [GitHub 导入 ruleset 文档](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/managing-rulesets-for-a-repository#importing-a-ruleset)。

该 ruleset 阻止对 `main` 的直接更新并要求通过 Pull Request，本身只匹配 `main`。实验 dev/stable/release 的命名及 PR 目标由本地和 CI 检查；这不代表远程 `Dev/LabX/main` 或 `LabX` 已部署保护规则。
