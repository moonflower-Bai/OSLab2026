## Why

Lab3（中断与异常）已成为下一个实验章节，但仓库里还没有该章节的代码骨架，`lab-status.json` 仍指向 `lab1`。需要把官方发布的 Lab3 代码作为 `lab3/` 导入，并把当前章节切换到 `lab3`，后续练习才能在 `Dev/Lab3/*` 分支上开发。

## What Changes

- 从 `http://oslab.mobisys.top/lab2026/lab3.zip`（解压目录 `lab3_new`）原样导入代码到 `lab3/`，不填写任何 `lab3 ... YOUR CODE` 位置。
- 在 `lab3/` 增加简短的 `AGENTS.md`，说明工具链、验证命令以及骨架尚未完成练习时无法编译的事实。
- 将 `lab-status.json` 的 `current` 改为 `lab3`，`frozen` 保持为空数组。
- 不引入构建产物（`bin/`、`obj/`）和 macOS 元数据文件。

## Non-goals

- 不实现 Lab3 的练习（`trapentry.S` 上下文保存/恢复、`trap.c` 异常处理等）。
- 不修改 `lab1/`、`lab2/` 等其他章节目录，也不整理 `README.md` 的进度说明。
- 不修改治理脚本、CI 或分支规则。

## Capabilities

### New Capabilities
- `lab3-skeleton`: 规定 Lab3 官方骨架的导入范围、完整性与未完成状态的可观察行为。

### Modified Capabilities

无。

## Impact

- 新增 `lab3/` 目录和 `lab3/AGENTS.md`；修改 `lab-status.json`。
- 之后治理检查要求实验代码改动只发生在 `lab3/`，与 `Dev/Lab3/*` 分支名一致。
