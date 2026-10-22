# Tasks

## 1. 测试套件

- [x] 1.1 在 `test/lab2/p2_blackbox/` 实现 D1 巡检器（含 5 种损坏状态自检）与 D2–D6 用例，AT-P2-01 至 AT-P2-14 全部落地；确认未引用 `free_list`、`nr_free` 或分配器 static 函数，未读取分配器实现。
- [x] 1.2 实现 `test/lab2/inject_p2.py` 与 `test/lab2/run_p2.sh`（全量一次启动、`--per-case`、`P2_PAGE_INIT_FROM`、`P2_MANAGER`），编写 `test/lab2/README.md`；运行后用 `git status --short lab2` 确认项目代码无改动。

## 2. 验证

- [x] 2.1 直接运行 `sh test/lab2/run_p2.sh`：当前分支编译失败（`pmm.c` 调用未定义的 `page_init`），日志保存为 `results/p2_blackbox_20261021-185900.log`。
- [x] 2.2 运行 `P2_PAGE_INIT_FROM=be87b20 sh test/lab2/run_p2.sh` 及 `--per-case`：30/30 通过，D1 自检 5 种损坏全部被发现，D5 三个种子各 1500 次操作通过；日志为 `results/p2_blackbox_20261021-185902.log` 与 `latest.log`。
- [x] 2.3 运行 `P2_MANAGER=default` 自检：23/30 失败，确认检查会报错。
- [x] 2.4 合并练习 1 的 `page_init()` 后，不借用直接运行 `sh test/lab2/run_p2.sh`，确认 30/30 通过。2026-10-22 在合并分支上复验：不设置 `P2_PAGE_INIT_FROM`，结果 30/30，日志 `test/lab2/results/p2_blackbox_20261022-000744.log`。

外部前提：RISC-V 交叉工具链、QEMU、OpenSBI。

补记说明（2026-10-21）：本 change 在测试完成后补建，任务 1.1–2.3 的勾选依据为原会话中的实施与运行记录。
