# Lab2 最终提示词

## 一、启动映射与物理页初始化（何闻硕）

````markdown
[任务]
阅读 lab2/kern/mm/pmm.c、pmm.h、memlayout.h、mmu.h、
kern/driver/dtb.c 和 kern/init/entry.S，完成 page_init()。

[约束]
1. 使用 get_memory_base/get_memory_size 获取 DTB 解析结果，不写死 128 MiB。
2. 区分虚拟地址、物理地址、物理页号和 Page 描述符地址。
3. 当前平台 DRAM_BASE 为 0x80000000，启动映射仅覆盖其后 1 GiB；
   对不支持或无效的范围终止初始化，不静默继续。
4. npage 是物理末端页号，Page 数量为 npage-nbase。
5. pages 放在 ROUNDUP(end, PGSIZE)，转换为物理地址后计算描述符占用。
6. 先将描述符全部初始化为保留，再将描述符之后的完整空闲页
   交给 init_memmap；固件、内核、描述符不得进入空闲区。
7. 检查描述符容量、对齐与非空区间，不登记零页或越界范围。
8. 检查切换 satp 后当前低地址 PC 是否仍可取指；必要时补充
   根表索引 2 的恒等映射，同时保留索引 511 的高地址映射。
9. 保留原有分配器自检和评分工具，不修改测试以使实现通过。

[验证]
编译并在默认 128 MiB、64 MiB 下启动，检查 DTB、页数、空闲区和
check_alloc_page()。若启动停在固件输出，使用 GDB 定位 satp 切换。
提交实现说明、真实日志，并解释根表索引和内存布局计算。
````

## 二、Best Fit（白景仁）

### 实现提示词

````markdown
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

### 黑盒测试提示词

````markdown
任务：创建test文件夹，根据项目配置，创建针对于如下方向的单元测试
  - D1 不变量巡检器：每次操作后扫描链表验证 B1–B5（这是对抗样例的公共基础设施）。
  - D2 最小块选择的边界：等尺寸多块并列、最小块位于链表头/尾、唯一块恰好等于请求。
  - D3 合并路径全组合：只前合、只后合、前后同时合、无邻不合；合并后再分配验证块完整性。
  - D4 状态污染与标志残留：分配/释放后扫描块内每一页的 `flags/property`。
  - D5 随机模型验证：以测试侧区间集合分别模拟 Best-Fit 的选择与合并，校验返回块、守恒和不重叠；不得与 First-Fit 锁步比较成功/失败，因为两种策略会合法地产生不同碎片状态。
  - D6 失败路径无副作用：`alloc_pages(n > nr_free)`、空链表 alloc 后验证 B1–B5 不变。

操作要求：你必须在项目中创建文件夹，同时不能修改原有代码，本次单元测试与项目代码保证隔离。仅允许读取函数调用方式，不得直接读取实现代码。本次单元测试的编写以黑盒测试的形式出现。

输出要求：其一：直接进行文件操作，但是文件操作仅允许在test文件夹内写入，项目文件夹内均可读。其二：将测试结果输出到Prompt里，但是不允许修改项目代码，即使测试失败。

可用函数接口记录在doc/lab2/p2_blackbox_api.md

单元测试样例参考：
*公共测试手法（全部样例共用）**：每个样例落地为一个新增 `static void at_p2_NN(void)`，不改动 `basic_check`/`best_fit_check` 本体，只由测试构建中的临时 `.check` 包装器调用。每个样例先从真实分配器申请 `SPAN+1` 页，前 `SPAN` 页是试验田，最后一页是 guard（防止零尺寸余块或 off-by-one 覆写真实空闲链表节点），再保存现场：

```c
struct Page *p0 = alloc_pages(SPAN + 1);    /* [p0,p0+SPAN) 试验田；p0+SPAN 为 guard */
assert(p0 != NULL);
list_entry_t free_list_store = free_list;
list_init(&free_list);
unsigned int nr_free_store = nr_free;
nr_free = 0;
```

合成状态用 `free_pages` 把 `[p0, p0+SPAN)` 的指定子区间挂入临时链表；未释放子区间与 guard 保持已分配。清场最多执行 `SPAN` 次 `alloc_pages(1)`，每次断言地址在试验田内且未重复；结束必须同时满足预期 `drained`、`list_empty(&free_list)`、`nr_free==0`，然后恢复链表头/计数并 `free_pages(p0, SPAN+1)`。D1 巡检器在解引用块区间前先检查节点数不超过 `SPAN`、块首落在试验田、`0<property<=SPAN-offset`、双链前后指针一致，再验证 B1–B5；损坏状态只在隔离镜像中失败停止，不能强行恢复。

### AT-P2-01 策略退化为 First-Fit（选首个足够块）
- **假设漏洞**：`alloc_pages` 直接复制 `default_pmm.c` 的遍历逻辑，遇到首个 `property >= n` 的块即停，而非遍历全程选最小足够块。
- **状态构造**：span = 6；`free_pages(p0, 3); free_pages(p0+4, 2);`（`p0+3` 保持已分配作隔离）→ `{3@p0, 2@p0+4}`，`nr_free == 5`。
- **操作序列**：`p1 = alloc_pages(2); p2 = alloc_pages(2);`
- **正确行为**：`p1 == p0+4`（2 页恰等块为最小足够块，整块摘下），链表仅剩 `{3@p0}`；`p2 == p0`（从 3 页块前部切分），余 `{1@p0+2}`；`nr_free` 依次 5→3→1。
- **漏洞特征**：First-Fit 下 `p1 == p0`（3 页块被切走，余 `{1@p0+2, 2@p0+4}`），`p2` 落到 `p0+4` 或 `p0+2`；返回值与余块形态均与 Best-Fit 不同。
- **检测断言**：`assert(p1 == p0 + 4); assert(nr_free == 3);`；遍历 `free_list` 断言仅剩一块且 `le2page(le, page_link) == p0 && p0->property == 3`；`assert(p2 == p0);` 再断言唯一余块首页 `== p0 + 2 && property == 1`。挂接：新增 `static void at_p2_01(void)`（保存-恢复与清场见本章开头），清场断言 `drained == 1`。
- **框架覆盖**：是（`best_fit_check` 中 `alloc_pages(1)` 必须返回 `p0+4` 已能区分两种策略），但框架未断言切分后的余块形态；本样例把策略区分与余块断言绑定，防止"选对块但切错"的混合漏洞逃逸。

### AT-P2-02 并列最小块一致性（两种选择都合法）
- **假设漏洞**：并列处理破坏链表或重复返回同一块；返回靠后块本身不是漏洞。
- **状态构造**：span = 11；`free_pages(p0, 2); free_pages(p0+4, 3); free_pages(p0+9, 2);`（`p0+2..p0+3`、`p0+7..p0+8` 隔离）→ `{2@p0, 3@p0+4, 2@p0+9}`，`nr_free == 7`。
- **操作序列**：`p1 = alloc_pages(2); p2 = alloc_pages(2);`
- **正确行为**：`{p1,p2}` 是 `{p0,p0+9}` 的任意排列且二者不同；两个 2 页块均被完整摘除，最终链表仅 `{3@p0+4}`，`nr_free == 3`。
- **漏洞特征**：两次返回相同地址、返回 3 页块、摘除后残留任一已分配块，或最终计数/链表不符。
- **检测断言**：`assert(p1 != p2); assert((p1==p0 || p1==p0+9) && (p2==p0 || p2==p0+9));`；最终遍历断言唯一块 `p0+4/property=3`。
- **框架覆盖**：否。`best_fit_check` 中最小块唯一（`{2@p0+1, 1@p0+4}`），并列情形从未出现。

### AT-P2-03 切分方向错误（从块后部切分）
- **假设漏洞**：切分时从块后部切出 `n` 页（返回 `page + property - n`，余块留在原首页），或余块偏移量算错。
- **状态构造**：span = 5；`free_pages(p0, 5);` → `{5@p0}`，`nr_free == 5`。
- **操作序列**：`p1 = alloc_pages(2);` → 探针 `free_pages(p1, 2); p2 = alloc_pages(5);`。
- **正确行为**：`p1 == p0`（前部切分）；余块为链表唯一块：首页 `p0+2`、`property == 3`、`PageProperty(p0+2) == 1`，块内 `PageProperty(p0+3) == PageProperty(p0+4) == 0`；探针释放后合并回 `{5@p0}`，`p2 == p0`。
- **漏洞特征**：后向切分下 `p1 == p0+3`、余块首页停留在 `p0`；偏移算错则余块首页/`property` 数值异常，探针的整并合验证暴露重叠或缺页。
- **检测断言**：`assert(p1 == p0);`；遍历得唯一块 `q`，`assert(q == p0 + 2 && q->property == 3 && PageProperty(q));`；`assert(!PageProperty(p0+3) && !PageProperty(p0+4));`；探针后 `assert(p2 == p0);`。挂接：`static void at_p2_03(void)`，探针结束时 5 页已全部取回 `p2` 手中，清场 `drained == 0`，恢复后直接 `free_pages(p0, 5)`。
- **框架覆盖**：否。`best_fit_check` 全部成功分配都是恰等整块（1==1、2==2、5==5），切分路径从未触发；`basic_check` 虽从 3 页块切 1 页，但对返回地址与余块位置零断言——后向切分实现可完整通过两个框架检查。

### AT-P2-04 切分余块链表位置错误（破坏 B1）
- **假设漏洞**：切分后的余块用 `list_add(&free_list, …)` 挂到链表头/尾，而非放回被切块的原链表位置。
- **状态构造**：span = 14；`free_pages(p0, 2); free_pages(p0+4, 5); free_pages(p0+11, 3);`（`p0+2..p0+3`、`p0+9..p0+10` 隔离）→ `{2@p0, 5@p0+4, 3@p0+11}`，`nr_free == 10`。
- **操作序列**：`p1 = alloc_pages(4);`（唯一满足块为 `5@p0+4`）→ 功能探针 `free_pages(p0+9, 2);`。
- **正确行为**：`p1 == p0+4`，余块 `{1@p0+8}` 占据原中间位置，链表首页序列 `[p0, p0+8, p0+11]` 严格升序；释放 `[p0+9,p0+11)` 后与左右相邻块合并为 `{6@p0+8}`，链表变 `{2@p0, 6@p0+8}`，`nr_free == 8`。
- **漏洞特征**：余块被挂到尾部 → 序列变 `[p0, p0+11, p0+8]`（B1 破坏）；探针合并依赖链表有序性，乱序实现漏并，得不到 6 页整块。
- **检测断言**：分配后断言首页/`property` 序列为 `[p0,p0+8,p0+11]` / `[2,1,3]`；探针后断言块序列 `[p0,p0+8]` / `[2,6]`、`nr_free==8`。清场 `drained == 8`。
- **框架覆盖**：否。同 AT-P2-03，框架从不触发切分，余块位置无任何断言。

### AT-P2-05 合并遗漏：只合前驱、漏后继
- **假设漏洞**：`free_pages` 只实现（或只生效）与前驱块的合并，漏掉与后继块的合并。
- **状态构造**：span = 4；先 `free_pages(p0+2, 2);` → `{2@p0+2}`。
- **操作序列**：`free_pages(p0, 2);`（新块只与后继相邻，前驱不存在）→ 探针 `p1 = alloc_pages(4);`。
- **正确行为**：合并为单块 `{4@p0}`：`p0->property == 4`、`PageProperty(p0) == 1`、被吸收的后继首页 `PageProperty(p0+2) == 0`、`nr_free == 4`；探针 `p1 == p0`。
- **漏洞特征**：残留 `{2@p0, 2@p0+2}` 两块（B5 破坏）；`alloc_pages(4)` 甚至 `alloc_pages(3)` 返回 `NULL`（应成功）。
- **检测断言**：遍历断言块数 `== 1` 且首页 `== p0`、`property == 4`；`assert(!PageProperty(p0 + 2));`；`assert(p1 == p0);`（探针取走后 `free_pages(p1, 4)` 复原再清场）。挂接：`static void at_p2_05(void)`，清场 `drained == 4`。
- **框架覆盖**：部分。`best_fit_check` 的释放要么落进空链表（`free_pages(p0,5)` 时链表已空），要么与邻块不相邻，后继合并路径从未在断言下执行过。

### AT-P2-06 合并遗漏：只合后继、漏前驱
- **假设漏洞**：`free_pages` 只实现与后继块的合并，漏掉与前驱块的合并。
- **状态构造**：span = 4；先 `free_pages(p0, 2);` → `{2@p0}`。
- **操作序列**：`free_pages(p0+2, 2);`（新块只与前驱相邻）→ 探针 `p1 = alloc_pages(4);`。
- **正确行为**：前驱吸收新块，首页仍为 `p0`：单块 `{4@p0}`，`p0->property == 4`、`PageProperty(p0+2) == 0`、`nr_free == 4`；探针 `p1 == p0`。
- **漏洞特征**：残留 `{2@p0, 2@p0+2}` 两块；探针失败（`p1 == NULL`）。
- **检测断言**：同 AT-P2-05 镜像：块数 `== 1`、`p0->property == 4`、`assert(!PageProperty(p0+2));`、`assert(p1 == p0);`。挂接：`static void at_p2_06(void)`，清场 `drained == 4`。
- **框架覆盖**：部分。理由同 AT-P2-05，前驱合并路径从未在断言下执行。

### AT-P2-07 合并遗漏：三明治场景只合一侧
- **假设漏洞**：新块前后都相邻时只合并一侧——典型写法是 `else if` 二选一，或合并前驱后仍用过期的 `base`/`property` 判断后继。
- **状态构造**：span = 3；`free_pages(p0, 1); free_pages(p0+2, 1);`（`p0+1` 隔离）→ `{1@p0, 1@p0+2}`。
- **操作序列**：`free_pages(p0+1, 1);` → 探针 `p1 = alloc_pages(3);`。
- **正确行为**：三合一为 `{3@p0}`：块数 `== 1`、`p0->property == 3`、`PageProperty(p0+1) == PageProperty(p0+2) == 0`、`nr_free == 3`；探针 `p1 == p0`。
- **漏洞特征**：残留 `{2@p0, 1@p0+2}` 或 `{1@p0, 2@p0+1}`；`alloc_pages(3)` 返回 `NULL`（应成功）；B5 破坏。
- **检测断言**：块数 `== 1` 且 `p0->property == 3`；`assert(!PageProperty(p0+1) && !PageProperty(p0+2));`；`assert(p1 == p0);`（取走后 `free_pages(p1,3)` 复原）。挂接：`static void at_p2_07(void)`，清场 `drained == 3`。
- **框架覆盖**：部分。`best_fit_check` 的 `free_pages(p0,5)` 落入空链表，前后同时相邻的三明治合并从未出现。

### AT-P2-08 无邻误合并（相邻性判断缺失）
- **假设漏洞**：合并时漏掉地址相邻条件（`p + p->property == base` / `base + base->property == p`），把不相邻块并成跨越已分配页的假块。
- **状态构造**：span = 4；`free_pages(p0, 1); free_pages(p0+2, 2);`（`p0+1` 隔离）→ `{1@p0, 2@p0+2}`，`nr_free == 3`。
- **操作序列**：构造后即巡检；`a = alloc_pages(3);`（总量够但分散，必须失败）→ 探针 `p1 = alloc_pages(1);`。
- **正确行为**：两块并存：首页序列 `[p0, p0+2]`、`property` 序列 `[1, 2]`、块数 `== 2`；`a == NULL` 且失败后状态不变；探针 `p1 == p0`（恰等 1 页块被 Best-Fit 选中），余 `{2@p0+2}` 不变。
- **漏洞特征**：被误并为 `{3@p0}` 单块（块数 `== 1`）；`alloc_pages(3)` 错误成功；或探针分配后余块首页变 `p0+1`——已分配的隔离页被卷进空闲块，后续双重分配/释放断言爆炸。
- **检测断言**：遍历断言首页序列 `[p0, p0+2]`、`property` 序列 `[1, 2]`、块数 `== 2`；`assert(a == NULL);` 后复检不变量；`assert(p1 == p0);` 随后断言唯一余块首页 `== p0+2` 且 `property == 2`。挂接：`static void at_p2_08(void)`，清场 `drained == 2`。
- **框架覆盖**：部分。`best_fit_check` 靠 `assert(alloc_pages(4) == NULL)` 间接挡住"误并后大块凭空出现"的一种形态，但从不直接断言块数与首页序列；跨已分配页的污染无法被框架发现。

### AT-P2-09 被吸收块标志残留 → 分配块内污染
- **假设漏洞**：合并吸收邻块时漏 `ClearPageProperty`，或 `free_pages` 未逐页清 `flags`——被吸收首页的 `PG_property` 残留为 1，成为块内"假头"。
- **状态构造**：span = 4；`free_pages(p0, 2); free_pages(p0+2, 2);` → 正确实现合并为 `{4@p0}`。
- **操作序列**：合并后立即检查 `p0+2` 标志；`p1 = alloc_pages(4);` 扫描 `[p1, p1+4)` 每页；`free_pages(p1, 4);`（释放本身即探针）。
- **正确行为**：合并后 `PageProperty(p0+2) == 0`；`p1 == p0` 且 4 页全部 `PageProperty == 0`（B6）；`free_pages(p1,4)` 的逐页断言 `assert(!PageReserved(p) && !PageProperty(p))` 顺利通过。
- **漏洞特征**：`p0+2` 的 `PG_property` 残留 → `alloc_pages(4)` 只清首页，返回块内存在 `Property == 1` 的页（直接命中"块内污染"）；`free_pages(p1,4)` 触发 panic；残留页后续还可能被误当空闲块头。
- **检测断言**：合并后 `assert(!PageProperty(p0 + 2));`；分配后 `for (i = 0; i < 4; i++) assert(!PageProperty(p1 + i));`；释放路径不 panic 即通过。挂接：`static void at_p2_09(void)`，清场 `drained == 4`。
- **框架覆盖**：否。框架检查从不触发真实合并（见 AT-P2-05/06/07），被吸收页与块内页的标志从未被观察；`assert(!PageProperty(p0))` 只验分配块首页。

### AT-P2-10 free 插入乱序（链表不按地址升序）
- **假设漏洞**：`free_pages` 把新块头插（`list_add(&free_list, …)`）或尾插，而非按地址有序插入；合并逻辑又依赖有序性，进一步失效。
- **状态构造**：span = 5，隔离页 `p0+1`、`p0+3` 保持已分配。第一轮（保存-重置后）升序释放：`free_pages(p0,1); free_pages(p0+2,1); free_pages(p0+4,1);`。第一轮清场后第二轮降序释放：`free_pages(p0+4,1); free_pages(p0+2,1); free_pages(p0,1);`。
- **操作序列**：每轮三次释放后巡检；第二轮末尾追加功能探针 `free_pages(p0+1,1); free_pages(p0+3,1); p1 = alloc_pages(5);`。
- **正确行为**：每轮链表首页序列都是 `[p0, p0+2, p0+4]` 严格升序（与释放顺序无关）；探针后五块合一，`p1 == p0`。
- **漏洞特征**：头插法在升序释放轮得到 `[p0+4, p0+2, p0]`，尾插法在降序释放轮同样逆序（B1 破坏）；乱序下合并漏并，探针 `alloc_pages(5)` 返回 `NULL`。
- **检测断言**：每轮用 `list_next` 遍历收集首页指针数组，断言逐元素等于 `{p0, p0+2, p0+4}`（兼验 B1）；探针 `assert(p1 == p0);`。挂接：`static void at_p2_10(void)`，span 在函数顶部取一次，两轮各自做保存-重置，第一轮清场 `drained == 3`，第二轮探针后 `drained == 0`（5 页已在 `p1` 手中），最后恢复并 `free_pages(p0, 5)`。
- **框架覆盖**：否。`basic_check` 的 `free_page(p0/p1/p2)` 恰为升序且随后即分配，头插/尾插乱序不影响其任何断言；框架无任何直接检查链表次序的断言。

### AT-P2-11 nr_free 漂移（切分减量/失败路径计数错误）
- **假设漏洞**：切分路径写成 `nr_free -= page->property`（减整块而非减 `n`），或失败路径误减，或释放路径重复累加。
- **状态构造**：span = 5；`free_pages(p0, 5);` → `{5@p0}`，`nr_free == 5`。
- **操作序列**：`p1 = alloc_pages(2);` → `p2 = alloc_pages(3);` → `free_pages(p1, 2);` → `p3 = alloc_pages(9);`。
- **正确行为**：`p1 == p0` 后 `nr_free == 3`（只减 `n`，余 `{3@p0+2}`）；`p2 == p0+2` 后 `nr_free == 0`、链表空；`free_pages(p1,2)` 后 `nr_free == 2`（`{2@p0}`）；`p3 == NULL` 且 `nr_free` 保持 2、块完整。
- **漏洞特征**：切分误减 `property` 时第一步后 `nr_free == 0` → `p2` 错误返回 `NULL`；失败路径误减时末尾 `nr_free != 2`；释放重复累加则计数虚高、B3 对账失败。
- **检测断言**：每步后 `assert(nr_free == 期望值 && nr_free_pages() == 期望值);`，并遍历断言各块 `property` 之和 `== nr_free`（B3）；`assert(p2 != NULL); assert(p3 == NULL);`。挂接：`static void at_p2_11(void)`，清场 `drained == 2`。
- **框架覆盖**：部分。`basic_check` 只断言 1 页整块操作下的 `nr_free == 3/0`，切分减量路径未覆盖；`best_fit_check` 有 `alloc_pages(4) == NULL` 但其后未断言 `nr_free` 数值，失败路径计数漂移不可见。

### AT-P2-12 失败路径副作用（含"总量够但分散"必失败）
- **假设漏洞**：缺 `n > nr_free` 提前返回或判定失败后仍改动状态；无单块满足时错误地跨块"拼凑"分配，或摘除/切分了部分块。
- **状态构造**：span = 7；`free_pages(p0, 2); free_pages(p0+4, 3);`（`p0+2..p0+3` 隔离）→ `{2@p0, 3@p0+4}`，`nr_free == 5`。
- **操作序列**：`a = alloc_pages(6);`（大于总量）→ `b = alloc_pages(5);`（等于总量但分散在多块，必须失败）→ `c = alloc_pages(4);`（小于总量但无单块 ≥ 4）→ 探针 `d = alloc_pages(3);`。
- **正确行为**：`a == b == c == NULL`；三次失败后 B7 成立：首页序列 `[p0, p0+4]`、`property` 序列 `[2, 3]`、严格升序、`nr_free == 5`，与构造时逐位一致；探针 `d == p0+4`（Best-Fit 选恰等的 3 页块）。
- **漏洞特征**：任一失败调用返回非 `NULL`（拼凑分配）；或失败后链表块被摘除/切分、`nr_free` 减少、次序变化；探针返回值偏离 `p0+4` 说明状态已被污染。
- **检测断言**：三次 `assert(返回值 == NULL);` 后逐项巡检（首页序列、`property` 序列、块数、`nr_free`、B1–B5）；`assert(d == p0 + 4);`。挂接：`static void at_p2_12(void)`，清场 `drained == 2`。
- **框架覆盖**：部分。`best_fit_check` 仅 `alloc_pages(4) == NULL` 一处失败断言（属 `n > nr_free` 早退路径），失败后只验 `p0[1].property == 2` 一个点；"等于总量但分散"与"小于总量无满足块"两类必失败语义框架均无。

### AT-P2-13 边界请求：n=1、恰等唯一块、零尺寸余块
- **假设漏洞**：`property == n` 的恰等情形仍走切分支，产生 `property == 0` 的零尺寸余块并 `SetPageProperty` 挂链（余块首页落在 `p0+n`，会篡改 span 之外真实内存页的 `flags`）；或 `n == 1` 极小请求路径特殊化出错。
- **状态构造**：span = 3；`free_pages(p0, 3);` → `{3@p0}`，`nr_free == 3`。
- **操作序列**：`p1 = alloc_pages(3);`（恰等唯一块）→ `a = alloc_page();`（空链表失败）→ `free_pages(p1, 3);` → `p2 = alloc_page();`（n=1 切分）→ `free_pages(p2, 1);`（合并复原）。
- **正确行为**：`p1 == p0` 后链表为空、`nr_free == 0`，不存在任何 `property == 0` 的残留块；`a == NULL`；释放复原后 `p2 == p0`、余块 `{2@p0+1}` 在链；最终合并回 `{3@p0}`。
- **漏洞特征**：恰等分配后 `list_empty(&free_list)` 为假、遍历到 `property == 0` 块；`p2 != p0` 或余块畸形则 n=1 路径出错。
- **检测断言**：`assert(p1 == p0); assert(list_empty(&free_list)); assert(nr_free == 0);`；遍历计数 `count == 0`；`assert(a == NULL);`；`assert(p2 == p0);` 并断言唯一余块首页 `== p0+1 && property == 2`。挂接：`static void at_p2_13(void)`，清场 `drained == 3`。
- **框架覆盖**：部分。`best_fit_check` 的 `alloc_pages(5)`/`alloc_pages(2)` 走恰等整块路径并断言 `nr_free == 0`，但从不断言 `list_empty` 与"无零尺寸块"；零尺寸块因 `property == 0` 可恰好通过 `total == nr_free` 对账而逃逸。

### AT-P2-14 init_memmap 乱序两段注册
- **假设漏洞**：`init_memmap` 假设只会被调用一次、或假设按地址递增注册——直接尾插/头插甚至重建链表，导致第二段丢失或链表乱序。
- **状态构造**：span = 16。保存-重置后，对 `[p0, p0+6)` 与 `[p0+8, p0+12)` 内每页先 `SetPageReserved`，然后降序调用 `best_fit_pmm_manager.init_memmap(p0+8,4); best_fit_pmm_manager.init_memmap(p0,6);`，不得绑定实现自定的 static 函数名。
- **操作序列**：两次注册后巡检；探针 `a = alloc_pages(7); b = alloc_pages(6);`。
- **正确行为**：链表 `{6@p0, 4@p0+8}` 严格升序，`nr_free == 10`；两段每页 `PageReserved == 0`、`ref == 0`，块首页 `PageProperty == 1`、`property` 分别为 6 和 4；探针 `a == NULL`（分散必失败）、`b == p0`。
- **漏洞特征**：第二段被丢弃（`nr_free != 10` 或遍历只剩一块）、逆序挂链（B1 破坏），或后注册段覆盖先注册段的 `page_link` 造成链表断裂/成环。
- **检测断言**：遍历断言首页序列 `[p0, p0+8]`、`property` 序列 `[6, 4]`、`nr_free == 10`；对两段内每页 `assert(!PageReserved(pg) && page_ref(pg) == 0);`；`assert(a == NULL && b == p0);`。挂接：`static void at_p2_14(void)`，清场 `drained == 4`（`b` 取走 6 页后剩 `4@p0+8`），恢复后 `free_pages(p0, 16)` 归还整段。
- **框架覆盖**：否。当前启动只注册单段；本样例属于多段注册鲁棒性检查，若课程验收只承诺单次注册则不作为基础合格门禁。
````

## 三、Buddy System（陈彦兴）

### 实现提示词

````text
你是一名操作系统内核开发者。请在 riscv64 ucore 风格的 Lab2 代码中,从零实现一个
Buddy System 物理页分配器,并接入现有 pmm_manager 框架。

背景与约束:
- 代码位于 lab2/kern/mm/;pmm.c 通过 struct pmm_manager 选择管理器。
- page_init() 已实现,会调用 pmm_manager->init_memmap(base, n),把一段已标记
  PageReserved 的连续空闲物理页交给管理器。
- struct Page 定义在 memlayout.h,包含 ref、flags、property、page_link;
  PageReserved/PageProperty/SetPageProperty/ClearPageProperty 等宏可用;
  全局 pages、npage、nbase 和 page2ppn() 可用。
- 不要修改 best_fit_pmm.c、default_pmm.c、page_init() 和现有测试函数;
  不要破坏默认管理器。
- 新增 kern/mm/buddy_pmm.c/.h,实现 struct pmm_manager 的
  init/init_memmap/alloc_pages/free_pages/nr_free_pages/check,
  并注册 buddy_pmm_manager。
- 通过 pmm.c 的 #ifdef USE_BUDDY_PMM 选择 Buddy;Makefile 用 BUDDY=1
  增加 -DUSE_BUDDY_PMM,默认构建保持不变。

算法要求:
- MAX_ORDER=15;每个 order 一条 free_area_t 空闲链表;空闲块首页用
  PageProperty 标记,property 保存 order;已分配块首页用私有 flag 位标记,
  property 保存分配时的 order。
- init_memmap:清除 PageReserved,把任意起始/长度的空闲区按地址对齐拆成
  2^k 块,分别登记到对应 order;维护每个 order 的计数和总 nr_free。
- alloc_pages(n):n=0 或 n>nr_free 返回 NULL;向上取整到 2^k;从最小可用
  order 取块,逐级对半拆分;返回对齐的块首;nr_free 按实际占用的块大小递减。
- free_pages(base,n):校验 base 是已分配块首;按保存的 order 归还整个 2^k
  块;与 buddy(页号 XOR 1<<order)逐级合并,直到 buddy 越界、不是同 order
  空闲块或达到 MAX_ORDER;nr_free 按归还块大小增加。
- check():验证每个 order 链表的块首、对齐、范围、property、PageProperty、
  块不重叠、没有未合并的同 order 相邻 buddy,块大小总和等于 nr_free;
  并执行基本的分配/释放/失败路径测试。
- 不使用需要 libgcc 的 __builtin_clz/ctz(-nostdlib 下会缺符号),用循环
  实现 log2/ctz;代码需通过 -Wall -Werror 编译。
- 说明关键不变量和复杂度。

输出:完整代码、pmm.c/Makefile 的最小改动、编译和运行步骤。
````

### 测试提示词

````text
请为上述 Buddy System 分配器设计一套可编译运行的测试,要求:

- 不修改现有 best_fit_check/default_check/评分脚本;
- 一个内核内 buddy_check(),作为 pmm_manager->check,在 QEMU 启动时由
  check_alloc_page() 调用;
- 一个主机端回归 lab2/tools/test_buddy_host.c,用 256 个 struct Page 和
  桩函数模拟内核环境,编译真实的 buddy_pmm.c,开启 ASan/UBSan。

覆盖范围:
1. 初始化:任意起始/长度拆成对齐块;每个 order 计数正确;总 nr_free 正确;
   块不重叠、地址对齐。
2. 分配:精确 2^k;非 2^k 向上取整(例如 5 页占 8 页块);拆分;跨 order
   选择;返回块对齐;nr_free 按块大小变化;n=0、n>nr_free、超过 MAX_ORDER
   的失败路径。
3. 释放/合并:buddy 已分配时不合并;buddy 空闲时合并;低地址/高地址 buddy;
   连续多级合并;释放后块首标记、property 和计数正确。
4. 随机:固定种子 2000 次分配/释放;测试侧用位图记录实际占用的 2^k 块,
   逐步断言 nr_free 和块不重叠;结束时归还全部页,恢复初始空闲页数。
5. 诊断:失败时打印用例名、页号、order、块大小和 nr_free;最后调用
   check() 验证不变量。

说明每个测试为什么能验证拆分、合并和边界行为。
````

## 四、SLUB 对象分配器（何闻硕）

### 实现提示词

````text
请阅读 lab2/kern/mm/pmm.h、memlayout.h、pmm.c、default_pmm.c、
libs/list.h 和 kern/init/init.c，在当前 ucore 框架中从零实现简化 SLUB。

目标是两层内存分配：底层继续调用现有 alloc_pages/free_pages，
上层通过 kmalloc(size_t size)、kfree(void *ptr) 分配任意正长度对象。
新增 slub_init，在 pmm_init 返回后初始化并接入独立自检。

请采用 16～2048 字节的 2 次幂大小缓存，返回地址至少 16 字节对齐。
每个小对象 slab 占一页，页内包含元数据及空闲对象链表；缓存跟踪
partial slab，分满时移出、释放后重新进入，最后一个对象归还时释放整页。
用槽位占用状态与所有活跃 slab 登记信息校验归还，不修改 struct Page。
大对象使用带头部的连续页申请，记录实际请求给底层的页数以便配对释放。

明确 kmalloc(0) 返回 NULL；检查 size 加头部的溢出；无法取得页时
返回 NULL 且不破坏存活对象、不泄漏页；kfree(NULL) 无动作。
非法对象指针与可检测的重复释放通过 panic 拒绝，不读取任意外来地址。
仅支持当前无并发调用场景，说明元数据、内部碎片和查找复杂度。

请编写易于 review 的 C 代码和接口注释，保留现有 pmm_manager 的选择，
不得修改 basic_check/default_check/best_fit_check 和原评分工具。
不要引入 libc、现成堆分配器或要求先实现 Buddy System。
````

### 测试提示词

````text
请阅读刚实现的 slub.h/slub.c 和 pmm 公共接口，独立设计测试，
保留原有 First Fit/Best Fit 自检与评分工具原文。

新增 slub_test.c，让内核启动能够执行对象层自检：
1. 覆盖每个大小档边界前后、1 字节、2048/2049、页大小前后和多页对象。
   同时持有多个对象，按请求长度写入不同数据，验证对齐、互不重叠和完整性。
2. 验证同类小对象共享页面；动态填满多个 slab，再释放整页及已满 slab
   中的一个槽位，验证 partial 状态恢复与槽位复用无需再分配页面。
3. 验证混合顺序释放后 nr_free_pages 恢复原值，不能只检查没有崩溃。
4. 覆盖 kmalloc(0)、kfree(NULL)、size_t 极值及溢出；实际耗尽底层页，
   检查新 slab 和大对象申请返回 NULL、原对象数据不变，而已有空闲槽仍可分配。
   归还所有持有页后应能重新正常分配。
5. 使用固定种子的混合长度申请/释放，独立维护存活对象范围和数据校验，
   不依赖分配器内部结构，也不强制未声明的地址选择顺序。

再写主机适配器，直接编译真实 slub.c/slub_test.c 和现有页分配器，
使用页对齐的模拟 RAM；在 ASan/UBSan 下执行更长的固定种子回归。
非法指针、对象内部指针、大对象内部指针和可检测的重复释放应在
独立进程中验证 panic，避免故意停止正常内核自检。
提供可复现的 Make 命令和日志，说明每组测试对应的潜在错误及覆盖局限。
````
