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
  - `0_environment_setup.md`、`3_startdash.md`：环境说明
- 尚未放入
  - `lab2/` 至 `lab9/`

## 开发规范

- 章节状态
  - 一章一个目录：`lab1/`、`lab2/`、……
  - 只修改 `lab-status.json` 指定的当前章节；冻结章节不可修改
  - 单章命令和工具链要求写在该章的 `AGENTS.md`
- 开发分支
  - 所有开发工作都在 `Dev/*` 分支上进行，例如 `Dev/fix-boot` 或 `Dev/lab1-notes`
  - 变更通过 Pull Request 合入 `main`；不要直接向 `main` 推送提交
  - 可导入的 GitHub `main` 保护规则见 [main-requires-pull-request.json](.github/rulesets/main-requires-pull-request.json)
- OpenSpec
  - 修改仓库前先创建 change；提案、实现、归档和提交分别授权
  - 根规则见 [AGENTS.md](AGENTS.md)，OpenSpec 配置见 [openspec/config.yaml](openspec/config.yaml)
  - 本地与 GitHub Actions 共用 `scripts/check_governance.py` 检查规格、范围、规则和 Prompt 门禁
- 用户 Prompt
  - 只保存直接影响 change 且不含敏感值的用户原文
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

在项目根目录运行 `openspec init`，并在交互界面选择正在使用的 AI 编程工具。也可用工具 ID 非交互配置，例如 `openspec init --tools codex,claude`；可用 ID 以 `openspec init --help` 为准。初始化或新增工具后，重启或重新加载 IDE/Agent。已有 OpenSpec 项目更新生成的工具指令时，在该项目根目录运行 `openspec update`。本仓库已包含 OpenSpec 配置，可直接使用其已有 change 和规格。

### 导入 main 分支保护规则

具有仓库管理员权限或 `edit repository rules` 权限的人员可将上述 JSON 导入 GitHub：打开仓库 **Settings → Rules → Rulesets**，选择 **New ruleset → Import a ruleset**，选中 JSON 文件，检查内容后点击 **Create**。导入后确认 ruleset 处于 Active 状态且目标是 `main`。详细步骤见 [GitHub 导入 ruleset 文档](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/managing-rulesets-for-a-repository#importing-a-ruleset)。

该 ruleset 阻止对 `main` 的直接更新并要求通过 Pull Request；`Dev/*` 是仓库开发约定，ruleset 本身只匹配 `main`。
