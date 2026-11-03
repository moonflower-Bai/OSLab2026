## 1. Implementation

- [x] 1.1 在 `lab3/kern/trap/trap.c` 中增加 `exception_handler()` 声明，实现 `IRQ_S_SOFT`、`IRQ_S_EXT` 和未知中断分支。
- [x] 1.2 增加 `IRQ_S_TIMER` 分支：`clock_set_next_event()`、`ticks++`、每 `TICK_NUM` 调用 `print_ticks()`。
- [ ] 1.3 增加 `<sbi.h>`、`SHUTDOWN_AFTER`，累计输出达到阈值后调用 `sbi_shutdown()`。
- [ ] 1.4 确认未修改 `trapentry.S`、`exception_handler()` 定义、lab2 代码、Makefile 和测试脚本。

## 2. Verification

- [ ] 2.1 `make -C lab3 clean && make -C lab3 obj/kern/trap/trap.o` 通过（每次代码提交后执行）。
- [ ] 2.2 在仓库外临时副本中补 lab2 `page_init`、`best_fit` free area、模块一 `RESTORE_ALL`/`__alltraps` 和 `exception_handler` 链接桩，使用旧汇编器包装构建。
- [ ] 2.3 运行官方 `GRADE_LAB3_TIMER` 用例，确认 `100 ticks`、`ticks=200` 和 PASS。
- [ ] 2.4 长跑确认累计 10 次 `100 ticks` 后 `sbi_shutdown()` 生效。
- [ ] 2.5 `openspec validate implement-lab3-interrupt-handler --strict` 和 `python3 scripts/check_governance.py ...` 通过。
