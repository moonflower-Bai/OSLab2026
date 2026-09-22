# Tasks

## 1. 约定与启动说明

- [x] 1.1 新增 `prompts/README.md`，写明路径、作者名来源和“不要读回历史提示”。完成后确认该文件存在且没有嵌入任何用户原文。
- [x] 1.2 把同一段保存规则写入 `AGENTS.md` 和 `CLAUDE.md`。完成后用比较确认两个文件逐字相同。
- [x] 1.3 在 `.cursor/rules/openspec.mdc` 中加入“先写入、不读其他提示”，并确认 `alwaysApply` 仍为 true。
- [x] 1.4 更新 `README.md` 和 `openspec/config.yaml`，使人和后续变更看到同一条约定。完成后确认启动说明里没有粘贴历史提示正文。

## 2. 按约定保存本条提示

- [x] 2.1 把本条用户 Prompt 写入 `prompts/repo/<author>/<date>/<time>-save-user-prompts.md`。完成后确认文件只含元数据和用户原文，且没有覆盖其他作者的文件。
