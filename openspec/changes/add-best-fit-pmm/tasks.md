# Tasks

## 1. 章节状态

- [x] 1.1 将 `lab-status.json` 的 `current` 改为 `lab2`，`frozen` 保持为空（仓库中没有 `lab1/` 目录，冻结它会让治理检查报“冻结章节目录不存在”）；运行 `python3 -B scripts/check_governance.py` 通过。

## 2. 实现

- [x] 2.1 在 `lab2/kern/mm/best_fit_pmm.c` 标记处实现 `free_area` 及 `free_list`、`nr_free` 宏、init、init_memmap、alloc_pages、free_pages、nr_free_pages 和按地址插入的辅助函数，文件末尾注册 `best_fit_pmm_manager`；用 `git diff lab2/kern/mm/best_fit_pmm.c` 确认唯一删除的行是标记行，两个测试函数未改动。
- [x] 2.2 将 `lab2/kern/mm/pmm.c` 的 `init_pmm_manager()` 改为 `pmm_manager = &best_fit_pmm_manager;`；用 `git diff lab2/kern/mm/pmm.c` 确认只改动这一行。

## 3. 验证

- [x] 3.1 主机端：用模拟头文件编译真实的 `best_fit_pmm.c`，运行原有 `best_fit_check()`，覆盖 alloc_pages(1) 返回 p0+4、alloc_pages(2) 返回 p0+1、free_pages(p0,5) 恢复 5 页块，并补一个 First-Fit 会选错块的用例，全部通过。
- [ ] 3.2 QEMU：完成练习 1 的 `page_init()` 后，在 `lab2/` 运行 `make qemu`，确认输出 `memory management: best_fit_pmm_manager` 和 `check_alloc_page() succeeded!`。2026-10-22 更新：合并分支已实现 `page_init()`，用 `-kernel` 启动已验证上述输出；`make qemu` 仍受本机 QEMU 7.0.0 的 fw_dynamic 限制（`-device loader` 不填 `next_addr`），待本机环境修复后再勾选。
- [x] 3.3 运行 `make grade`，确认 `tools/test_lab2.c` 的 `test_best_fit` 通过。2026-10-22 在合并分支上通过，20/20；本机用 PATH 包装层把 `-device loader` 转成 `-kernel`，仓库命令未改。

外部前提：RISC-V 交叉工具链、QEMU 与可用的 OpenSBI。3.2 仍受本机 QEMU 固件交接方式限制；3.3 已通过，证据见 `lab2/report/logs/best-fit-grade.log`。

补记说明（2026-10-21）：本 change 在实现完成后补建，任务 1.1–3.1 的勾选依据为原会话中的实施与验证记录。
