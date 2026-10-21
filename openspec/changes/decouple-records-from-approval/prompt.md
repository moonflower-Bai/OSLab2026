# User Prompts

change: decouple-records-from-approval
reviewed_for_relevance: true
reviewed_for_sensitive_data: true

## 2026-10-21 18:00:00 +0800

现在这个项目之前我openspec因为介入太多，减少了一些，但是现在看来减少太多了，没有AI遵守openspec了

## 2026-10-21 18:05:00 +0800

我的本意是用户说直接实现，不需要在完成OpenSpec之后停下来等用户说可以实现了，而是先完成OpenSpec的文档以及要求，自检无误之后直接开始实现

## 2026-10-21 18:20:00 +0800

PR检查没有意义。AI回头补OpenSpec的意义不大。实验2就这样了。你开个Dev/*分支做，做完gh直接合并到main。下次开Lab3的实验的时候再补了。而且也不需要所有工具都补，三个人现在估计下来就Agentsmd和claudemd两类，harness就claude code，deepseek harness ，codex和hermes四个

## 2026-10-21 19:05:00 +0800

woc，这个感觉已经变成某种状态机了。你以状态机的视角说明openspec的适用范围，以免prompt变成某个酒吧里的炒饭。然后openspec的修改还是在刚才的分支，如果git被占用，就写在这里但是不提交，等待另一个操作结束再迁移
