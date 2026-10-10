# Spec Delta

## Purpose

规定操作系统实验任务来源、材料完成状态和课程目录导出的可审阅行为，使成员能够区分官方要求、小组规范与建议分工，并把完整源码、报告、提示词和真实截图整理为指定交付结构，避免缺项被隐藏或既有材料被覆盖。

## ADDED Requirements

### Requirement: Trace requirements to their sources
实验要求清单 MUST 标明课程交付通知、报告模板、指导书题目和仓库规则各自来源。未取得正文的题目与仅依据本地缓存的进展 MUST 标为未核对，建议分工 MUST NOT 被描述为最终分工。

#### Scenario: Only the guide overview is available
- **WHEN** 本地只有概览正文和练习页链接
- **THEN** 清单给出原链接并将具体题意列为待核对

### Requirement: Explain individual contribution
个人材料 MUST 区分课程骨架、队友材料和本人的新增工具或修改，并给出复现命令与实际验证限制。MUST NOT 为了形成提交记录把未改动的骨架描述为个人实现。

#### Scenario: The member contributes verification tools
- **WHEN** 成员新增启动和调试验证工具但未修改内核功能
- **THEN** 报告与提交范围明确描述这些工具及其证据，不声称重写内核

### Requirement: Export the required course layout
交付导出入口 MUST 在完整输入具备时生成 code/、report/report.md、report/prompt.md 和 report/images/。代码 MUST 来自当前章节，报告引用的本地截图 MUST 存在；源码和报告输入 MUST 保持不变。

#### Scenario: Complete input is provided
- **WHEN** 当前章节源码、报告、合法课程 Prompt 和引用截图齐备
- **THEN** 新交付目录包含指定结构和对应文件，并报告导出位置

### Requirement: Reject incomplete or unsafe exports
导出入口 MUST 拒绝缺失必需文件、缺失引用截图、未审阅的课程 Prompt 和会覆盖已有内容的目标，并列出原因。默认目标 MUST 位于仓库外新目录，MUST NOT 自动创建 Git commit、分支、push 或 PR。

#### Scenario: An image referenced by the report is absent
- **WHEN** 报告引用的本地测试截图不存在
- **THEN** 导出返回非零状态并报告缺失路径，不产生可被误认为完整交付的结果

#### Scenario: The destination already contains files
- **WHEN** 用户选择了已有内容的目标目录
- **THEN** 导出拒绝覆盖并保持原有文件不变

### Requirement: Exclude local tool and build artifacts
课程代码导出 MUST 排除本机下载的软件包、工具安装目录、node_modules、生成的 obj/bin 及临时调试文件；课程要求的报告截图 MUST 被保留。导出完成 MUST NOT 代表尚未核对的题目已经通过验收。

#### Scenario: A local build has already produced binaries
- **WHEN** 当前章节包含构建生成文件且报告引用测试截图
- **THEN** 导出代码不包含这些构建产物，报告中仍包含所引用截图
