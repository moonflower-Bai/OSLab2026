# Spec Delta

## ADDED Requirements

### Requirement: Allow bounded course prompt artifacts
课程要求的 Prompt 汇总 MUST 被视为单独交付材料，仅允许当前章节的 report/prompt.md 草稿与根 report/prompt.md 交付路径。内容 MUST 由用户明确提供，包含实验标识和人工相关性、敏感性审阅声明；MUST NOT 自动读取其他 change 或遗留会话补齐内容，MUST NOT 改变 OpenSpec 证据只保存在当前 change 的规则。

#### Scenario: Reviewed course prompts are supplied
- **WHEN** 用户明确提供本实验相关且不含敏感值的 Prompt，并完成审阅声明
- **THEN** 合法课程路径可以通过门禁，其他任意 Prompt 路径仍被拒绝

#### Scenario: A course prompt contains sensitive content
- **WHEN** 课程 Prompt 匹配敏感信息模式或缺少审阅声明
- **THEN** 检查拒绝该文件并报告位置，不保存脱敏或删节副本

#### Scenario: Only old change evidence is available
- **WHEN** 当前课程汇总缺少用户明确提供的内容
- **THEN** 系统报告缺少输入，不读取或迁移其他 change 的 Prompt
