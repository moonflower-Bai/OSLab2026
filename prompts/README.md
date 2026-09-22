# change 级 Prompt 证据

Prompt 证据用于说明 OpenSpec change 的需求来源，不是聊天记录备份。证据只放在对应 change 内：

```text
openspec/changes/<change-name>/prompt.md
openspec/changes/archive/<date>-<change-name>/prompt.md
```

## 收录规则

只有同时满足以下条件的用户消息才可原文收录：

1. 直接决定当前 change 的需求、范围、设计取舍、验收标准或授权；
2. 不含姓名、学号、身份证号、账号凭证、密钥、token 或其他敏感值；
3. 已由记录者人工检查相关性和敏感性。

普通问答、状态询问、社交内容和与实现无关的上下文不保存。不得为了“完整还原对话”收录无关消息，也不得读取或迁移其他 change 或遗留会话文件来补齐上下文。

只要相关消息含任何敏感值，整条原文都不得保存。不要保存脱敏版、删节版、摘要或改写版；只写不含原文内容的省略记录：

```markdown
## <timestamp>

omitted: sensitive-input
```

## 文件格式

```markdown
# User Prompts

change: <change-name>
reviewed_for_relevance: true
reviewed_for_sensitive_data: true

## <timestamp>

<用户原文>
```

每条原文使用带时区的时间标题。文件不得包含助手回复、工具输出或从其他对话补写的内容。自动检查只能识别有限模式，不能代替保存前的人工判断。
