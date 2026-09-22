# OSLab2026

## 指导书

[riscv64-ucore 操作系统实验指导书](http://8.135.34.58/lab2026/_book/)

- [lab0 预备起](http://8.135.34.58/lab2026/_book/lab0/intro.html)
  - 环境、工具链、QEMU
- [lab0.5 AI 驱动的操作系统实验](http://8.135.34.58/lab2026/_book/lab0.5/intro.html)
  - 提示词规格与 AI 协作方式
- [lab1 最小可执行内核](http://8.135.34.58/lab2026/_book/lab1/lab1.html)
- lab2 至 lab9
  - [lab2 物理内存和页表](http://8.135.34.58/lab2026/_book/lab2/lab2.html)
  - [lab3 中断](http://8.135.34.58/lab2026/_book/lab3/lab3.html)
  - [lab4 进程管理](http://8.135.34.58/lab2026/_book/lab4/lab4.html)
  - [lab5 用户程序](http://8.135.34.58/lab2026/_book/lab5/lab5.html)
  - [lab6 进程调度](http://8.135.34.58/lab2026/_book/lab6/lab6.html)
  - [lab7 同步互斥](http://8.135.34.58/lab2026/_book/lab7/lab7.html)
  - [lab8 文件系统](http://8.135.34.58/lab2026/_book/lab8/lab8.html)
  - [lab9 页面置换与内存映射](http://8.135.34.58/lab2026/_book/lab9/lab9.html)
- [附录](http://8.135.34.58/lab2026/_book/appendix/intro.html)

## 开发进度

`lab-status.json` 是章节状态的唯一事实来源。当前章节为 `lab1`，冻结章节为空。

- 已放入仓库
  - `lab1/`：最小内核骨架，当前练习是阅读启动流程；构建与运行要求见 [lab1/AGENTS.md](lab1/AGENTS.md)
  - `0_environment_setup.md`、`3_startdash.md`：环境说明
- 尚未放入
  - `lab2/` 至 `lab9/`

## 开发规范

- 章节状态
  - 一章一个目录：`lab1/`、`lab2/`、……
  - 只修改 `lab-status.json` 指定的当前章节；冻结章节不可修改
  - 单章命令和工具链要求写在该章的 `AGENTS.md`
- OpenSpec
  - 修改仓库前先创建 change；提案、实现、归档和提交分别授权
  - 根规则见 [AGENTS.md](AGENTS.md)，OpenSpec 配置见 [openspec/config.yaml](openspec/config.yaml)
  - 本地与 GitHub Actions 共用 `scripts/check_governance.py` 检查规格、范围、规则和 Prompt 门禁
- 用户 Prompt
  - 只保存直接影响 change 且不含敏感值的用户原文
  - 证据放在对应 change 的 `prompt.md`，不提交按会话生成的归档
  - 细节见 [prompts/README.md](prompts/README.md)
