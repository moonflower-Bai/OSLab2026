# Proposal

## Why

现有仓库包含 Lab1 原始内核骨架，构建、启动和 GDB 的前置验证材料却保存在仓库外；本地尚无课程要求的 lab1 交付分支及完整的 code/report 材料。需要把可复现验证、个人贡献和任务依据整理进仓库，并解决课程 report/prompt.md 与现有路径门禁的冲突，方便代码审阅和答辩。

## What Changes

- 在当前 lab1 目录新增环境检查、有限时长的启动验证和 GDB 启动验证脚本，生成可追溯的输出；把这组工具及验证说明作为个人开发提交的具体范围。
- 新增 Lab1 操作说明、要求与完成状态清单、答辩知识点和个人验证材料，明确正式题目已确认项与外部待核对项。
- 提供交付导出工具，将当前章节代码和用户明确提供的完整报告材料整理为 code/、report/report.md、report/prompt.md、report/images/；输入不足时失败并列出缺项，默认导出到仓库外新目录。
- 对课程交付 Prompt 增加明确、有限的路径例外，允许当前章节 report/prompt.md 草稿及根 report/prompt.md 交付文件；保留其他 Prompt 路径、相关性、敏感信息和人工审阅约束。
- 对照已有 LaTeX 草稿记录待整合项，保留队友已有工作和开发分支；最终课程分支的创建、Git commit、push、PR 和规格归档继续分别授权。

范围：实验相关工具、材料仅位于 lab1；导出、治理、规格和仓库级入口属于工作流元数据。

非目标：本 change 不凭空确定练习二或 Challenge 的题意，不为了形成个人提交而改动内核，不安装或提交工具二进制，不编造截图、AI 迭代经历或评分通过结果，不自动公开仓库或合并队友分支，不读取或迁移其他 change 的 Prompt。

## Capabilities

### New Capabilities

- `lab1-validation`: 环境、构建、启动和 GDB 观察具有可复现命令、明确退出状态和相应证据。
- `lab-submission-delivery`: 任务来源与完成状态可审阅，完整课程材料可导出为指定目录结构，缺失输入得到明确报告。

### Modified Capabilities

- `prompt-archive`: 增加课程交付 Prompt 的有限路径与格式例外，保持 OpenSpec change 证据的保存边界。

## Impact

- 预计新增 lab1/scripts/、lab1/README.md、lab1/docs/、lab1/report/ 和小体积文本验证证据；构建产物仍由现有 .gitignore 排除。
- 预计新增 scripts/export_lab1_submission.py，并修改 scripts/check_governance.py、AGENTS.md、prompts/README.md、openspec/config.yaml；根 README 只增加入口链接。
- 依赖已核对的 RISC-V GCC/Binutils、Make、QEMU、GDB、Python 和 OpenSpec；默认使用标准工具命令，不依赖个人绝对路径或仓库外包装脚本。
- 主规格 prompt-archive 尚未同步已完成的 restrict-prompt-archive change；本提案按当前 AGENTS.md 与该 delta 的实际规则设计，不授权归档既有 change，归档时须检查二者兼容性。
- 外部前提：Lab1 练习及报告要求页全文、最终成员分工与 AI 使用记录、真实终端截图、课程相关 Prompt 和公开仓库状态。当前远程访问失败，不能把本地缓存当作远端最新状态。
