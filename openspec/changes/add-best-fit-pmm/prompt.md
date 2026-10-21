# User Prompts

change: add-best-fit-pmm
reviewed_for_relevance: true
reviewed_for_sensitive_data: true

## 2026-10-21 16:13:45 +0800

[PROMPT]
**任务**：修改文件 lab2/kern/mm/best_fit_pmm.c，在 `//lab2 function2-2 YOUR CODE` 标记位置实现完整的 Best-Fit 连续物理内存分配模块；并修改 lab2/kern/mm/pmm.c 的 init_pmm_manager()，将当前物理内存管理器切换为 best_fit_pmm_manager。
**操作要求**：你必须在项目中修改实际文件。best_fit_pmm.c 中已提供的 basic_check() 与 best_fit_check() 两个测试函数及其内的任何代码都不得修改、移动或删除；保留 `//lab2 function2-2` 标记注释（`YOUR CODE` 替换为本人学号占位 `<STUDENT_ID>`），实现代码插入标记位置（即两个测试函数之前）。pmm.c 中只许改动 init_pmm_manager() 内的管理器选择一行。
**输出要求**：直接进行文件操作。以 kern/mm/default_pmm.c 的 First-Fit 实现为结构参考，但分配策略必须是 Best-Fit。使用下面 [RELY]、[GUARANTEE]、[SPECIFICATION] 的信息，不引入无关功能。
[RELY]
// 接口契约（kern/mm/pmm.h）：struct pmm_manager { const char *name; void (*init)(void); void (*init_memmap)(struct Page *base, size_t n); struct Page *(*alloc_pages)(size_t n); void (*free_pages)(struct Page *base, size_t n); size_t (*nr_free_pages)(void); void (*check)(void); }
// 数据结构（kern/mm/memlayout.h）：struct Page { int ref; uint64_t flags; unsigned int property; list_entry_t page_link; }；free_area_t { list_entry_t free_list; unsigned int nr_free; }
// 页标志宏：SetPageProperty/ClearPageProperty/PageProperty、PageReserved 等
// 链表 API（libs/list.h）：list_init/list_add/list_add_before/list_add_after/list_del/list_empty/list_next/list_prev；le2page(le, page_link)
// 页转换（pmm.h）：page2pa、pa2page、page_ref、set_page_ref；全局 npage、PGSIZE、PPN
// 已有参考实现：kern/mm/default_pmm.c（first-fit 的完整框架，含 static free_area_t free_area 与 #define free_list/#define nr_free）
// 硬约束：本文件中的 best_fit_check() 直接引用 free_list 与 nr_free 两个名字，实现必须在测试函数之前定义 static free_area_t free_area 及宏 #define free_list (free_area.free_list)、#define nr_free (free_area.nr_free)
// 注册声明：kern/mm/best_fit_pmm.h 已声明 extern const struct pmm_manager best_fit_pmm_manager
[GUARANTEE]
必须实现的接口（均为 static，命名自定但语义对应）并定义注册结构：
```c
const struct pmm_manager best_fit_pmm_manager;  // .name="best_fit_pmm_manager"，.check=best_fit_check（使用文件中已有函数）
```
包含：初始化内部状态；按初始空闲空间建立描述符与空闲块；分配至少 n 个连续页；释放 n 个连续页并合并；返回空闲页数。
你可以根据需要添加：static 辅助函数与内部宏；不得修改两个测试函数；不得改变 pmm.h 的公开接口。
[SPECIFICATION]
## best_fit_init / best_fit_init_memmap
**Pre-Condition**: init_memmap 的调用者保证 base 起的 n 页全部处于 Reserved 状态且 n>0。
**Post-Condition**: 内部空闲链表为空→含一个按地址有序插入的空闲块；该区间每页 flags/property/ref 清零，base->property=n 且 SetPageProperty(base)；nr_free 增加 n。
## best_fit_alloc_pages
**Pre-Condition**: n>0。
**Post-Condition**: 返回满足 property>=n 的**最小**空闲块的首页描述符；无满足块或 n>nr_free 时返回 NULL。
**Case 1**（块大于 n）：从该块前部切出 n 页，剩余部分保持 property 与 Property 标志并留在链表原位置，nr_free-=n，返回块的 ClearPageProperty 首页。
**Case 2**（块等于 n）：整块摘下，nr_free-=n，返回首页。
**Case 3**（无可满足块）：返回 NULL，空闲链表与 nr_free 不变。
**Requirements**: 选择语义是"所有足够块中尺寸最小者"（best-fit），与 first-fit 的"首个足够块"相区别；多个同尺寸最小块时任取其一。
## best_fit_free_pages
**Pre-Condition**: n>0，base 起的 n 页非 Reserved 且非 Property（即已分配状态）。
**Post-Condition**: n 页全部清零 flags/ref，base->property=n 并置 Property，按地址有序插入空闲链表，nr_free+=n；与前驱块（p+p->property==base）和后继块（base+base->property==p）分别合并为更大块，合并后仅大块首页保留 Property 与正确 property。
## 整体验收
- make qemu 启动输出 `memory management: best_fit_pmm_manager` 并 `check_alloc_page() succeeded!`；
- best_fit_check 的关键场景必须成立：空闲块 {size2 @p0+1, size1 @p0+4} 中 alloc_pages(1) 返回 p0+4、alloc_pages(2) 返回 p0+1；free_pages(p0,5) 后恢复为完整 5 页块；
- tools/test_lab2.c::test_best_fit 通过（pmm_manager 指向 best_fit_pmm_manager）。
````

## 2026-10-21 16:14:11 +0800

切到 lab2 并冻结 lab1 (Recommended)
