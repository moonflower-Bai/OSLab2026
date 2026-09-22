# User Prompts

change: restrict-prompt-archive
reviewed_for_relevance: true
reviewed_for_sensitive_data: true

## 2026-09-22 21:17:21 +0800

隐私泄露问题……我在考虑要不要做成提交前的Github Action，但是好像并不能改变本质。但是另一个角度，这是个私有仓库，Prompt是用于提交作业的。直接保存确实有点问题，但是不保存又无法从OpenSpec还原原始Prompt。你先给个解决方案再说。至于其他问题，我都同意并且你可以立刻执行修改

## 2026-09-22 21:25:35 +0800

不保存敏感原文，而且这是作业，理论上最敏感的就是姓名学号这些，API Key不会在这里保存。但是依旧是含有敏感信息的原文不保存，只保存与项目实现有直接关联的Prompt。Github Action也做。脱敏就不用了。

## 2026-09-22 21:32:16 +0800

应用两个change
