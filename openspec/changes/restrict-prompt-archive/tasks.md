# Tasks

## 1. 切换 Prompt 保存规则

- [x] 1.1 更新 `AGENTS.md`、Claude/Cursor 入口和 `openspec/config.yaml`，只在消息直接影响当前 change 且不含敏感信息时保存原文，并用规则检查确认不再要求每条消息先写会话文件。
- [x] 1.2 重写 `prompts/README.md`，定义相关性判断、敏感消息整条拒绝、`prompt.md` 格式和省略标记，并检查文档中没有脱敏副本或完整会话归档要求。
- [x] 1.3 更新 `.gitignore`，让按会话生成的 Prompt Markdown 保持本地且不进入新提交，同时确认 `prompts/README.md` 仍可被 Git 跟踪。

## 2. 建立 change 级 Prompt 证据

- [x] 2.1 为 `harden-ai-development-governance` 创建 `prompt.md`，只收录本会话中直接决定该 change 且不含敏感值的用户原文，并检查不含助手回复或工具输出。
- [x] 2.2 为 `restrict-prompt-archive` 创建 `prompt.md`，收录隐私决策原文和人工检查声明，并检查内容没有姓名、学号或其他实际敏感值。
- [x] 2.3 停止继续追加当前会话文件，不读取或迁移其他会话文件；用 Git 状态确认遗留会话文件不会被纳入提交。

## 3. 增加提交门禁

- [x] 3.1 扩展治理检查脚本，拒绝被跟踪的会话归档、不允许的 Prompt 路径、脱敏占位符以及带字段和值的姓名、学号、身份证和常见凭证模式，并用安全与违规样例验证退出状态。
- [x] 3.2 让 GitHub Actions 对 pull request 和 push 调用 Prompt 门禁，确认检查失败时只报告位置而不改写文件。
- [x] 3.3 检查每个新 change 的 `prompt.md` 具有 change 标识、时间和人工相关性/敏感性声明，并确认 CI 不把扫描通过描述为完整隐私保证。

## 4. 验证规格与最终范围

- [x] 4.1 运行两个 active change 的严格校验和全部主规格校验，确认 `prompt-archive` delta 与治理规格没有冲突。
- [x] 4.2 运行完整治理检查并审查 Git diff，确认没有保存敏感原文、没有修改内核实现、没有读取或改写其他会话文件。
- [x] 4.3 报告自动检测的能力边界和遗留本地文件状态，等待用户分别授权归档两个 change。
