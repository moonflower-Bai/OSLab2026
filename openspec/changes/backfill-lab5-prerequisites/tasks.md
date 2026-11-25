# Tasks

## 1. 迁入前置实现

- [x] 1.1 获得章节切换授权后将 `lab-status.json.current` 改为 `lab5`，保留冻结列表。
- [x] 1.2 核对各实验稳定分支，恢复 Lab2 物理页管理、Lab3 异常与中断分发、Lab4 页表操作与内核线程和调度，实现时适配 Lab5 已有接口。
- [x] 1.3 完成陷阱入口调用、S/U 态 `RESTORE_ALL` 及 `forkrets` 共享返回路径。

## 2. 验证

- [x] 2.1 使用 RISC-V GCC 和 binutils 执行 `make -C lab5 -j2`，记录独立练习留下的阻塞。检查汇编目标和反汇编的寄存器、CSR 与陷阱帧偏移。
- [x] 2.2 运行 `sh openspec/changes/backfill-lab5-prerequisites/verify-trap.sh`，用 QEMU virt 与 OpenSBI 限时验证页表自检、内核线程切换、定时中断以及构造的 S/U 态陷阱往返。记录测试驱动、实际运行命令与结果。
- [x] 2.3 执行 `openspec validate backfill-lab5-prerequisites --strict`、`python3 -B scripts/check_governance.py --target Lab5` 和 `git diff --check`。
