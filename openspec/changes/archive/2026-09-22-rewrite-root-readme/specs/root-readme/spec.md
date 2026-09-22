# Spec Delta

## Purpose

规定根目录说明先呈现指导书层次和仓库进度，再呈现跨章节规范，并且不收录某一个实验的构建步骤。

## ADDED Requirements

### Requirement: Guide outline comes first
根目录 `README.md` MUST 以指导书的层次结构开头。该部分 MUST 给出指导书入口，以及各章在书中的位置。它 MUST NOT 用一段自然语言代替这个层次。

#### Scenario: A reader opens the repository
- **WHEN** 读者打开根目录 `README.md`
- **THEN** 第一个内容章节是指导书层次，而不是实验进度或某一章的构建命令

### Requirement: Progress is the second section
开发进度 MUST 是指导书层次之后的下一节。进度 MUST 只说明仓库里各章的状态，MUST NOT 夹带某一章的构建或调试步骤。

#### Scenario: A reader looks for current status
- **WHEN** 读者在指导书层次之后继续阅读
- **THEN** 下一节列出当前已放入仓库的章节和尚未放入的章节

### Requirement: Shared conventions follow progress
跨章节的开发规范 MUST 排在开发进度之后，并且先于任何单章操作说明。规范 MUST 覆盖章节目录的隔离、OpenSpec 留痕和用户 Prompt 的保存方式。

#### Scenario: A reader looks for how the group works
- **WHEN** 读者读完开发进度
- **THEN** 下一节是适用于整个仓库的开发规范

### Requirement: Single-lab build steps stay out
根目录 `README.md` MUST NOT 包含某一个实验的编译、运行或调试命令。

#### Scenario: The current chapter is lab1
- **WHEN** 仓库里只有 lab1 的骨架
- **THEN** 根目录说明不出现 `make`、`make qemu` 或 `make gdb`
