# Proposal

## Why

根目录 `README.md` 把指导书、进度、lab1 构建命令和各项规范按写作顺序堆在一起，读的人要翻完才能找到约定。总说明应先给出指导书的层次，再给出仓库进度，然后是跨章节都适用的规范。

## What Changes

- 重写根目录 `README.md` 的章节顺序和写法。
- 去掉单一实验的构建、运行和调试步骤。
- 开发规范放在进度之后，不再散落在文末。

## Capabilities

### New Capabilities

- `root-readme`: 根目录说明的信息顺序，以及它不收录单章构建步骤。

### Modified Capabilities

- 无。

## Impact

- 只改 `README.md`。
- 不改 `AGENTS.md`、各章目录或构建脚本。
