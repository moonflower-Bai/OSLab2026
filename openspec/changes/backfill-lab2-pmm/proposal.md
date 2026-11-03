## Why

Lab3 官方骨架已经导入，但指导书要求先把 Lab2 的物理内存管理成果填入 `lab2` 标记位置。当前 `lab3/kern/mm/pmm.c` 缺少 `page_init()`，`lab3/kern/mm/best_fit_pmm.c` 缺少 Best-Fit 主体和管理器实例，尚不能提供 Lab2 已完成的物理内存管理能力。

## What Changes

- 以 `origin/Dev/Lab2/main` 中的 Lab2 实现为源，在 `lab3/kern/mm/pmm.c` 回填基于 DTB 的物理内存探测、页描述符初始化和空闲页登记。
- 保留 Lab3 的 `local_intr_save` / `local_intr_restore` 分配保护及既有启动页表处理，不整份覆盖 Lab2 文件。
- 在 `lab3/kern/mm/best_fit_pmm.c` 回填空闲块管理、最佳适配分配、释放合并和管理器实例，保持官方 `basic_check()`、`best_fit_check()` 不变。
- 将 Lab3 默认物理内存管理器切换为已回填的 `best_fit_pmm_manager`。

## Non-goals

- 不实现 `trapentry.S` 或 `trap.c` 中的 Lab3 练习。
- 不导入 Buddy 分配器或新增 Lab3 骨架中不存在的内存管理模块。
- 不修改官方检查函数、测试注入脚本、构建系统或其他章节目录。
- 不归档 change、提交或推送分支。

## Capabilities

### New Capabilities

- `lab3-pmm-backfill`: 规定 Lab2 物理内存管理成果在 Lab3 骨架中的回填范围、适配约束和可验证行为。

### Modified Capabilities

无。

## Impact

- 修改 `lab3/kern/mm/pmm.c` 和 `lab3/kern/mm/best_fit_pmm.c`。
- 新增本 change 的规划、规格和 Prompt 证据。
- Lab3 的 Trap 练习仍未实现，因此完整内核链接、QEMU 和 grade 仍需后续 Lab3 change 才能完成。
