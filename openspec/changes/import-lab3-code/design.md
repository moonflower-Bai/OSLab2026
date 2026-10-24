## Context

官方 Lab3 骨架解压后目录名为 `lab3_new`，包含 `Makefile`、`kern/`、`libs/`、`tools/`。Lab1、Lab2 已采用“章节目录直接放实验代码”的布局。已验证：骨架直接 `make` 会因缺少 `exception_handler` 而在 `kern/trap/trap.c` 失败，这是练习尚未实现所致。

## Goals / Non-Goals

**Goals:**
- 原样导入官方代码，使后续 diff 只反映练习实现。
- 让 `lab-status.json`、分支名与目录三者一致。

**Non-Goals:**
- 不修复骨架编译失败，不实现练习。

## Decisions

- 目录命名为 `lab3/`，与现有 `lab1/`、`lab2/` 一致，而不是保留 `lab3_new`。
- 不修改任何官方文件，包括让骨架可编译的“占位实现”：占位会掩盖练习缺失，也会污染与官方版本的对比。
- `lab3/AGENTS.md` 只补充章节级构建与验证说明，不放宽根规则。
- 从解压目录复制时排除 `.DS_Store`、`__MACOSX`、`bin`、`obj`。

## Risks / Trade-offs

- 骨架无法编译，CI 若将来加入构建检查会失败 → 当前 CI 只运行治理检查，且 `AGENTS.md` 明确说明。
- 官方压缩包日后可能更新 → 在 tasks 中记录下载来源，后续差异另开 change 处理。
