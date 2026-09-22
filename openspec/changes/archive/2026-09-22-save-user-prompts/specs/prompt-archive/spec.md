# Spec Delta

## Purpose

把每一条用户 Prompt 存成独立文件，供多人以后查阅，同时不让这些历史提示进入 AI 的自动上下文。

## ADDED Requirements

### Requirement: Save each user prompt
处理一条新的用户 Prompt 时，AI MUST 先把该条原文写入 `prompts/`，再做其他仓库修改。只回答问题且不改文件时，也 MUST 保存这条 Prompt。

#### Scenario: A new user message arrives
- **WHEN** 会话收到一条新的用户 Prompt
- **THEN** `prompts/` 下出现一个只属于这条 Prompt 的新文件，正文是用户原文

### Requirement: Keep each prompt in its own file
每条 Prompt MUST 单独占用一个文件。路径 MUST 按作用域、作者、日期分开。AI MUST NOT 改写他人目录里已有的提示文件，也 MUST NOT 把多条提示追加进同一个文件。

#### Scenario: Two people save prompts on the same day
- **WHEN** 两个作者在同一天各保存一条提示
- **THEN** 两条提示位于不同作者目录下的两个文件，互不覆盖

### Requirement: Keep a prompt file self-contained
每个提示文件 MUST 只包含这条 Prompt 的最小元数据（作者、时间、作用域）和用户原文。文件 MUST NOT 写入助手回复、工具输出或其他提示。

#### Scenario: Someone opens one prompt file later
- **WHEN** 只打开某一个提示文件
- **THEN** 不借助聊天记录也能看出是谁、在什么范围、问了什么

### Requirement: Do not load the archive into context
AI MUST NOT 为了获取上下文而列举、搜索或阅读 `prompts/` 中除 `prompts/README.md` 和本条刚写入的文件以外的内容。启动时读入的说明 MUST NOT 嵌入历史提示正文。

#### Scenario: A later session starts
- **WHEN** 新会话开始，仓库里已经有多条历史提示
- **THEN** 这些历史提示的正文不会进入该会话的自动上下文
