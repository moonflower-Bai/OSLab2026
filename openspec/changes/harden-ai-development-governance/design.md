# Design

## Context

当前仓库同时面向多种 AI 工具，根规则、平台规则和 OpenSpec 生成的 skills 存在多份入口。见 proposal.md 的 Why。当前只有 `lab1/`，没有机器可读的章节状态、章节局部指令或 CI workflow；已有规范文件还混用了 CRLF 行尾。

## Goals / Non-Goals

**Goals:**

- 让所有工具从同一个仓库级状态机得到一致结论，同时保留各平台自己的命令语法。
- 让“当前章节”和“冻结章节”能够被脚本验证，而不是靠自然语言猜测。
- 让代码修改、工作流元数据和提交授权成为三个不同边界。
- 为 lab1 提供不会无限等待的基础验证路径。

**Non-Goals:**

- 不修改 OpenSpec 生成的各平台 skill 正文。
- 不修改内核代码或补充后续实验章节。
- 不在本 change 中决定 Prompt 明文、密文或提交范围。

## Decisions

### `AGENTS.md` 是仓库级规则的唯一正文

`CLAUDE.md` 只使用 Claude 支持的 `@AGENTS.md` 导入语法，不再复制正文。Cursor 的 always-apply rule 保留为短引导，只要求遵守根规则并使用 Cursor 自己可发现的 OpenSpec workflow。其他支持 `AGENTS.md` 的工具直接加载根文件。

备选是继续同步两份全文。它依赖人工或 CI 比较，仍会让修改者面对两个权威文件。

### 用 `lab-status.json` 表示章节状态

根目录新增 `lab-status.json`，包含一个 `current` 字段和一个 `frozen` 数组。初始状态为当前章节 `lab1`、冻结章节为空。切换章节时先更新该文件；治理检查据此限制实验实现路径。

备选是从目录名或最新 Git commit 推断。二者都不能表达“已放入但尚未提交”和“已经课程提交”的区别。

### 章节命令下沉到 `lab1/AGENTS.md`

根 README 继续只描述仓库级结构。`lab1/AGENTS.md` 记录工具链前提、`make -C lab1` 构建命令、QEMU 长运行边界和成功输出；自动验证只使用确定会结束的构建命令，不把交互式 QEMU 当作默认 CI 步骤。

### 规划、实现、归档和提交分别授权

根规则采用明确状态机：新修改先 propose；新一轮明确 apply 后实现；验证完成后等待 archive；只有用户明确要求时才执行 Git commit。这样与 OpenSpec skills 的授权边界一致。

### 一个治理检查入口供本地和 CI 共用

新增脚本读取 `lab-status.json`，运行 OpenSpec 严格校验，检查冻结章节 diff、规则入口和 LF 行尾。GitHub Actions 只调用同一入口，避免本地与远端出现两套判断。

Prompt 泄露检查不塞进这个入口；待 Prompt 分层方案确认后再单独设计，避免把“扫描通过”误当作“保存明文是安全的”。

## Risks / Trade-offs

- [Claude 版本不支持 `@AGENTS.md` 导入] → 在本机现有版本上做最小加载验证，并保留回退说明。
- [CI 无法判断与基线相比哪些冻结文件变化] → 脚本接受显式基线参数，CI 使用 pull request base，本地缺少基线时只做静态检查并明确提示。
- [QEMU 运行不会自行退出] → 默认门禁只编译；交互式运行说明必须包含超时或退出方式。
- [一次性规范化行尾产生较大 diff] → 只规范本 change 实际触及的治理文件，并通过 `.gitattributes` 固定后续行为。

## Migration Plan

1. 先引入单一规则入口、章节状态和章节局部说明。
2. 再增加本地治理脚本和 CI，并让 README 只描述已经存在的检查。
3. 运行 OpenSpec 严格校验、治理检查和 lab1 构建。
4. 验证通过后等待用户授权归档；若需要回滚，移除新增入口并恢复原有规则文件，不影响内核代码。
