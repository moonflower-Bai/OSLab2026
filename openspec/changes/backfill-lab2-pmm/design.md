## Context

Lab3 骨架在 `pmm.c` 和 `best_fit_pmm.c` 各保留一个 `lab2` 回填标记。最终 `origin/Lab2` 的 `code/kern/mm/` 与 `origin/Dev/Lab2/main` 的 `lab2/kern/mm/` 中，两份对应文件的 blob 哈希分别一致，因此使用后者作为源不会丢失发布分支内容。

Lab2 与 Lab3 的 PMM 外壳存在刻意差异：Lab2 默认选择 Best-Fit 并可选 Buddy，但没有 Lab3 的中断保护包装；Lab3 没有 Buddy 源文件，且已加入 `sync.h` 保护。不能直接覆盖整个 `pmm.c`。

## Goals / Non-Goals

**Goals:**

- 恢复 Lab2 已验证的 DTB 物理内存初始化和 Best-Fit 行为。
- 保留 Lab3 的中断安全和启动页表集成。
- 保持官方分配器检查函数不变。

**Non-Goals:**

- 不完成 Trap 练习，不增加 Buddy，不改变分配器接口或测试标准。

## Decisions

1. **按功能片段回填，而非整文件复制。** `pmm.c` 只迁移 `page_init()` 并选择 `best_fit_pmm_manager`；保留 Lab3 的 include、带中断保护的 `alloc_pages()`、`free_pages()`、`nr_free_pages()` 和 `pmm_init()` 页表部分。
2. **不迁移 Buddy 条件编译。** 当前 Lab3 没有 `buddy_pmm.h/c`，该功能也不在两处 `lab2` 标记的必要范围内；固定使用 Lab2 的默认 Best-Fit 管理器。
3. **Best-Fit 主体与检查函数分片段合入。** 在官方检查函数之前加入空闲区状态、按地址插入、初始化、最佳适配分配、前后块合并及计数实现，在检查函数之后加入 `best_fit_pmm_manager`；检查函数正文保持官方原样。
4. **内存布局沿用 Lab2 的 DTB 实现。** 使用 `get_memory_base()` / `get_memory_size()` 确定内存范围，在内核 `end` 后页对齐放置页描述符，保留固件、内核和元数据页，空闲区域经页对齐后交给 `init_memmap()`。
5. **验证分层。** 先编译两个 PMM 对象确认回填可编译，再尝试完整 `make -C lab3`。若链接仅因尚未实现的 Trap handler 失败，则如实记录为后续 Lab3 练习的阻塞，不通过临时桩代码绕过。

## Risks / Trade-offs

- 页描述符范围或物理/虚拟地址换算错误会导致启动期断言或内存损坏 → 保留 Lab2 已验证的边界检查和对齐逻辑，并核对 Lab3 的 `va_pa_offset`、DTB 与 `PADDR` 接口一致。
- 直接选择 Best-Fit 会改变骨架的 First-Fit 默认值 → 这是恢复 Lab2 默认行为并实际执行 Best-Fit 自检所必需，且骨架已包含 `best_fit_pmm.h`。
- 完整构建仍会被 Trap 空实现阻塞 → 本 change 只声称 PMM 编译验证，不声称 QEMU/grade 已通过。
