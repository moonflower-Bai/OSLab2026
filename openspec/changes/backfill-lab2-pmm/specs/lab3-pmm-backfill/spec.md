## ADDED Requirements

### Requirement: Lab2 physical memory initialization is backfilled into Lab3

`lab3/kern/mm/pmm.c` MUST 提供与 Lab2 一致的 DTB 物理内存初始化流程，包括内存范围读取、页描述符 placement、保留页初始化、空闲区对齐和 `init_memmap()` 登记，同时 MUST 保留 Lab3 已有的中断保护与启动页表处理。

#### Scenario: Initialize memory from the firmware DTB

- **WHEN** 内核调用 `pmm_init()`
- **THEN** PMM 使用 DTB 报告的物理内存范围创建并初始化页描述符，保留固件、内核和元数据区域，并将对齐后的空闲物理页交给当前管理器

#### Scenario: Preserve Lab3 integration behavior

- **WHEN** 审阅 `alloc_pages()`、`free_pages()`、`nr_free_pages()` 和 `pmm_init()` 的 Lab3 特定部分
- **THEN** 分配相关入口仍使用 `local_intr_save` / `local_intr_restore`，启动页表的 `satp_virtual` / `satp_physical` 处理仍然存在

### Requirement: Best-Fit allocator is restored and selected

`lab3/kern/mm/best_fit_pmm.c` MUST 提供完整的 Best-Fit 管理器实现，`lab3/kern/mm/pmm.c` MUST 选择 `best_fit_pmm_manager`。分配 MUST 选择满足请求的最小空闲块，释放 MUST 支持相邻前块和后块合并，空闲页计数 MUST 与链表内容一致。

#### Scenario: Allocate from the smallest sufficient block

- **WHEN** 空闲链表中存在多个容量足够的块并请求连续页面
- **THEN** 返回满足请求的最小块；若块大于请求则保留剩余部分并正确更新空闲计数

#### Scenario: Free and coalesce adjacent blocks

- **WHEN** 释放的块与现有空闲块在物理页描述符序列中相邻
- **THEN** 按地址维护链表并合并相邻前块或后块，块属性和空闲页总数保持正确

#### Scenario: Run the official allocator checks

- **WHEN** `pmm_init()` 调用当前管理器的 `check()`
- **THEN** 执行的是官方 `basic_check()` 和 `best_fit_check()`，其函数正文未被回填过程修改

### Requirement: Backfill remains within the Lab2 dependency scope

回填 MUST NOT 实现 Lab3 Trap 练习、引入 Buddy 分配器或修改其他章节。Lab3 未完成的 Trap handler MUST NOT 用临时实现规避。

#### Scenario: Inspect the final change set

- **WHEN** 检查本 change 的实验代码 diff
- **THEN** 只有 `lab3/kern/mm/pmm.c` 和 `lab3/kern/mm/best_fit_pmm.c` 被修改，且没有 Trap 桩实现、Buddy 文件或检查函数改动
