# Spec Delta

## REMOVED Requirements

### Requirement: Keep each prompt in its own file
**Reason**: 同一组对话被拆成多个文件后，压缩上下文时无法判断后续提示属于哪一组。
**Migration**: 把同一会话号下的用户 Prompt 追加进 `prompts/<author>/<session-id>.md`。

## MODIFIED Requirements

### Requirement: Save each user prompt
处理一条新的用户 Prompt 时，AI MUST 先把它追加进当前会话文件，再做其他仓库修改。只回答问题且不改文件时，也 MUST 保存这条 Prompt。当前会话文件不存在时，AI MUST 创建它。

#### Scenario: A new user message arrives
- **WHEN** 会话收到一条新的用户 Prompt
- **THEN** 该会话文件中增加这一条用户原文，且没有为此另建一个只含这一条的文件

### Requirement: Keep a prompt file self-contained
一个会话文件 MUST 包含作者、会话号、开始时间，以及该组对话中每一条用户 Prompt 和它的时间。文件 MUST NOT 写入助手回复或工具输出。

#### Scenario: Someone opens one prompt file later
- **WHEN** 只打开某一个会话文件
- **THEN** 不借助聊天记录也能看出这是谁的哪一组对话，以及其中每一条用户原文

### Requirement: Do not load the archive into context
AI MUST NOT 为了获取上下文或寻找所属对话而列举、搜索或阅读其他会话文件。启动时读入的说明 MUST NOT 嵌入历史提示正文。AI MAY 只打开由本次重新计算出的会话号确定的那一个文件，以便追加。

#### Scenario: A later session starts
- **WHEN** 新会话开始，仓库里已经有其他对话文件
- **THEN** 那些文件的正文不会进入该会话的自动上下文

## ADDED Requirements

### Requirement: Group one conversation into one file
同一外部会话号下的用户 Prompt MUST 写入同一个文件 `prompts/<author>/<session-id>.md`。不同会话号 MUST 使用不同文件。AI MUST NOT 修改其他作者目录中的文件。

#### Scenario: Two people talk at the same time
- **WHEN** 两个作者各有一组对话
- **THEN** 记录落在两个作者目录下的两个文件里，互不追加

### Requirement: Resolve the conversation outside compressed context
每次保存前，AI MUST 从进程环境，或该环境指向的会话目录，重新计算会话号。AI MUST NOT 用已被压缩的聊天记录判断这条 Prompt 属于哪一组对话。没有外部会话号时，AI MUST 新建文件，且 MUST NOT 追加到最近修改的其他对话文件。

#### Scenario: Context was compressed
- **WHEN** 同一组对话的后续消息到达，且先前的聊天记录已被压缩
- **THEN** AI 仍把这条 Prompt 追加进该会话号对应的文件

#### Scenario: No external session key
- **WHEN** 进程环境没有可用的会话号
- **THEN** AI 新建一个文件，不读取目录来猜测上一组对话
