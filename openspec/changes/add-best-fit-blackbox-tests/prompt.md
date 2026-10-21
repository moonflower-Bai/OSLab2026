# User Prompts

change: add-best-fit-blackbox-tests
reviewed_for_relevance: true
reviewed_for_sensitive_data: true

## 2026-10-21 18:44:12 +0800

[PROMPT]
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
