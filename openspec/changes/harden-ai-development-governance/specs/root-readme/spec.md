# Spec Delta

## ADDED Requirements

### Requirement: Automation claims match repository state
根目录 `README.md` 对 CI、自动检查和当前章节的陈述 MUST 能由仓库中的配置或机器可读状态文件验证。尚未实现的自动化 MUST 明确标为计划，而不能描述成现有能力。

#### Scenario: Reader checks an automation claim
- **WHEN** README 声称某项 CI 或自动检查已经启用
- **THEN** 仓库中存在对应配置，且该配置引用的当前章节与机器可读状态一致
