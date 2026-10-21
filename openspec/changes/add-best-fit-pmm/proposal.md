# Proposal

## Why

Lab2 练习 2 要求在 `best_fit_pmm.c` 中实现 Best-Fit 连续物理内存分配，并让内核使用它替代默认的 First-Fit 管理器。本 change 为已完成的实现补记规划记录：实现发生在 `decouple-records-from-approval` 规则生效之前，当时没有创建 change。

## What Changes

- 在 `lab2/kern/mm/best_fit_pmm.c` 的 `//lab2 function2-2` 标记处实现 Best-Fit 物理内存管理器：初始化、建立初始空闲块、分配、释放并合并、查询空闲页数，并注册 `best_fit_pmm_manager`（`.check` 使用文件内已有的 `best_fit_check`）。
- 标记注释中的 `YOUR CODE` 替换为学号占位 `<STUDENT_ID>`。
- `lab2/kern/mm/pmm.c` 的 `init_pmm_manager()` 只改管理器选择一行，切换为 `best_fit_pmm_manager`。
- `lab-status.json` 的 `current` 改为 `lab2`，`frozen` 保持为空。

非目标：不修改 `basic_check()`、`best_fit_check()` 两个测试函数；不改变 `pmm.h` 公开接口；不实现练习 1 的 `page_init()`（`//lab2 function1`）；不引入与 Best-Fit 无关的功能。

## Capabilities

### New Capabilities

- `lab2-best-fit-pmm`：Lab2 内核以 Best-Fit 策略管理连续物理页的分配与释放。

### Modified Capabilities

无。

## Impact

- 实验代码：`lab2/kern/mm/best_fit_pmm.c`、`lab2/kern/mm/pmm.c`。
- 章节状态：`lab-status.json`。
- 依赖练习 1 的 `page_init()` 完成后，内核才能编译并在 QEMU 中运行验收。
