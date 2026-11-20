# Lab4 提示词整理

本文整理 Lab4 开发中使用的 Prompt 体系。
---

## 一、虚拟内存页操作（白景仁）

### 1.1 单元测试与三层验收设计 Prompt

````markdown
[Prompts]

本次实验要实现 Lab4 功能模块一的虚拟内存页操作。请先为 `lab4/kern/mm/pmm.c` 的 `page_insert()` 和 `page_remove()` 编写宿主机单元测试、QEMU 集成测试和模型仿真测试，并交付独立的一键运行脚本。参考所需的 `lab4/kern/` 和 `lab4/libs/` 接口。测试调用仓库中的实际实现，可用测试桩控制物理页分配失败和观察 TLB 失效。当前接口尚未实现时，明确报告对应测试无法通过的原因。

[测试目标]

1. **宿主机单元测试**
   - 覆盖首次映射、同一物理页映射到多个虚拟地址、原址原页更新权限、原址换页、撤销最后一条映射和重复删除。检查叶 PTE、物理页、权限、引用计数和回收。原址原页更新权限期间，该页不得被释放。仍有别名时，删除一条映射不得回收共享页。
   - 分别删除从未建立页表路径的地址和路径存在但叶项无效的地址。两种情况都不应分配页、改变空闲页数或影响已有映射。
   - 在空页表上分别注入第一次和第二次中间页表分配失败。`page_insert()` 须返回 `-E_NO_MEM`，目标地址无有效叶映射，已有映射和目标数据页的引用不变，解除故障后可以重试。允许复用 `get_pte()` 已挂接的空中间表，在测试清理时单独核对这些表页。

2. **QEMU 集成测试**
   - 实际访存验证换页。给两块物理页写入不同标记，先映射到 A 并读一次，再替换为 B。通过同一虚拟地址立即读到 B，旧页的另一条映射仍读到 A。测试代码在换页与读取之间不得主动刷新 TLB。
   - 实际访存验证降权与撤销。先通过可写映射写入，再降为只读。读取应成功，写入应触发 store page fault，原数据保持不变。删除映射后，读取应触发 load page fault。测试专用 Trap 入口检查 `scause` 和故障地址，并恢复到后续测试。
   - 使用测试专用启动代码和链接脚本，链接实际被测代码。访存用例须使用合法的 Sv39 权限与页表路径，允许预建中间页表以单独检查叶项操作。QEMU 必须实际启动、完成断言并输出可识别的结束结果。

3. **模型仿真测试**
   - 用独立模型维护 `(页表, 虚拟页号) → (物理页, 权限)`，从映射条数推导测试数据页的引用数。模型不得复用被测实现的查表或引用计数逻辑。
   - 至少用 4 个固定种子，每个种子执行 256 步映射、换页、权限更新、删除和查询。覆盖同页别名、不同页表使用相同虚拟地址，以及 2 MiB 和 1 GiB 边界两侧的 4 KiB 页。每步比较实际状态与模型，失败时输出种子、失败步骤和可重放的操作序列。
   - 加入可控的 TLB 仿真，让已缓存的翻译持续有效，直到被测实现发出适用的失效操作。以此稳定检出换页、降权或撤销后遗漏失效的错误。

[一键运行与验收]

- 从仓库根目录执行 `bash lab4/test/run_vmem_tests.sh`，按单元测试、QEMU 集成测试、模型仿真测试的顺序运行。某层失败后继续执行其余可运行的层。
- 每项输出「通过」「错误」或「未运行」。错误项输出测试层、用例名和实际观察结果。最后汇总三层状态。工具缺失、编译或链接失败、断言失败、QEMU 超时均须说明原因并返回非零。只有三层均实际运行且通过时才返回 0。
- 在测试临时副本中验证三种错误变体：原址原页重复增加引用、最后一条映射删除后漏回收、换页后漏做 TLB 失效。每个变体应至少被对应的一项测试识别。默认入口始终验证仓库中的实际实现，变体仅用于验证测试有效性。

[不变量]

- 只允许写入 `lab4/test/` 和本次 change 的 OpenSpec 目录。测试源码、脚本、链接配置和构建产物放在 `lab4/test/` 下，清理范围限于本次生成的临时文件。
- 不允许读取 `lab4/tools/`，也不允许通过现有评分或构建入口间接读取它。测试使用自己的构建和运行入口。
- 保持主程序行为不变。内核源码、主 Makefile、现有自检与评分规则均保持原样。本阶段交付测试、独立运行脚本和对应的 OpenSpec 记录。
````

### 1.2 正式实现 Prompt

````markdown
[Prompts]

请在 `lab4/kern/mm/pmm.c` 的 `LAB4 function1 YOUR CODE` 处实现虚拟内存页操作。使用现有 Sv39 页表框架、Lab2 的物理页分配器和 `struct Page` 引用计数。保留 `pmm.h` 声明的接口，按上面设计的验收测试验证已有实现。

```c
int page_insert(pde_t *pgdir, struct Page *page, uintptr_t la, uint32_t perm);
void page_remove(pde_t *pgdir, uintptr_t la);
```

[功能要求]

1. **建立或更新映射。** `page_insert()` 使用 `get_pte(pgdir, la, true)` 定位或创建末级页表项，成功返回 `0`。首次映射时记录目标物理页号、调用方本次传入的权限和 `PTE_V`，目标页引用增加一次。若原映射已经指向同一物理页，更新权限但维持原引用计数，该页始终保持已分配。若原映射指向另一物理页，新页引用增加一次，旧页引用减少一次。旧页引用降至零后才交还物理页分配器。
2. **处理分配失败。** 若 `get_pte()` 无法提供末级页表项，返回 `-E_NO_MEM`。不得留下有效叶映射、增减目标数据页引用或改变已有映射。可以保留 `get_pte()` 已挂接的空中间表供后续重试。
3. **撤销映射。** `page_remove()` 以不创建页表的方式查找末级项。若页表路径不存在或末级项无效，直接返回。若存在有效映射，清除该项并减少旧页引用，引用降至零后回收数据页。中间页表的回收由其拥有者负责。
4. **同步地址翻译。** 建立、更新或删除有效映射时，确保 PTE 更新先于对应的 TLB 失效。若操作可能回收旧数据页，应先使旧翻译失效，再让该页重新可分配。参考现有的 `tlb_invalidate()` 和 `flush_tlb()`，并考虑编译器对页表写入与 `sfence.vma` 的排序。

[实现范围与验证]

- 实现文件限于 `lab4/kern/mm/pmm.c` 中的两项页操作及必要的同文件辅助逻辑。复用 `get_pte()`、`pte_create()`、`pte2page()` 和已有的引用计数接口。保留函数签名、物理页分配策略、现有自检和评分规则。测试 Prompt 的写入限制只适用于测试阶段。
- 不允许读取 `lab4/tools/`，包括通过主构建或评分入口间接读取。用 `bash lab4/test/run_vmem_tests.sh` 验证单元、QEMU 集成和模型仿真三层结果。已知的其它实验模块构建问题应单独报告。
- 先完成本次实现的 OpenSpec 记录并通过严格校验。在交付时说明首次映射、同页更新、替换、删除及内存不足五种路径如何维持 PTE、引用计数和 TLB 一致，并报告实际运行的测试结果。
````

---

## 二、进程控制块与线程创建（何闻硕）

````markdown
[任务]
依据 Lab4 指导书任务二与 OpenSpec 设计，在 `lab4/kern/process/proc.c` 中实现 `alloc_proc`、`copy_thread`、`do_fork`、`kernel_thread` 以及 `proc_init`。

[约束与规范]
1. `alloc_proc`：申请 PCB 并完整 memset 零初始化；状态设为 PROC_UNINIT，pid 设为 -1，pgdir 设为 boot_pgdir_pa，初始化 list_link 和 hash_link。
2. `copy_thread`：在分配好的内核栈顶预留 trapframe 并拷贝入参 tf；设置 a0 为 0（子线程返回值约定）；关键是将 context.ra 指向 forkret，context.sp 指向 trapframe 首地址。
3. `do_fork`：完整串联 alloc_proc -> setup_kstack -> copy_mm -> copy_thread。进入关中断临界区检查进程上限（nr_process < MAX_PROCESS），分配全局唯一 PID，挂入 proc_list 与 hash_list，最后调用 wakeup_proc 置为 PROC_RUNNABLE。如果任何步骤失败，必须以严格逆序释放已分配资源（bad_kstack -> bad_proc）。
4. `proc_init`：初始化 proc_list 与 hash_list；手工构造 0 号 idleproc（复用 bootstack 与 boot_pgdir，need_resched=1，设为 current）；调用 kernel_thread 创建 1 号 initproc（执行 init_main）。

[验收]
单独编译 proc.o 无 warning；运行隔离测试通过，且 `test_lab4_process_create()` 断言全部通过。
````

---

## 三、进程调度与上下文切换（陈彦兴）

````markdown
[任务]
在 `lab4/kern/process/switch.S` 中实现 `switch_to`，并在 `lab4/kern/process/proc.c` 与 `lab4/kern/schedule/sched.c` 中实现 `proc_run`、`cpu_idle` 与 `schedule`。

[约束与规范]
1. `switch_to`：
   - 输入参数 a0 为 from 进程 context 指针，a1 为 to 进程 context 指针。
   - 依次保存 ra, sp, s0~s11 共 14 个寄存器到 a0 中。
   - 从 a1 中依次恢复 ra, s0~s11，最后恢复 sp。
   - 执行 ret 跳转到目标线程 context.ra。
2. `proc_run`：关中断保护，更新 current 指针，写入 64 位 Sv39 satp 寄存器，在页表变化时执行 sfence.vma，调用 switch_to。
3. `cpu_idle`：死循环监测 current->need_resched 触发 schedule()。
4. `schedule`：关中断，从 current 下一节点开始回绕遍历循环链表 proc_list，跳过哨兵节点，选择首个 PROC_RUNNABLE 线程；若无可用线程回退至 idleproc，更新 runs 计数后执行 proc_run。

[验收]
`test_lab4_schedule()` 多线程交替运行通过，官方 `make grade` 获得 30/30 满分。
````

---

## 四、前置依赖回填 Prompt

### 4.1 Lab2 物理内存管理（PMM）回填

````markdown
[任务]
将 Lab2 已完成并验证的基于设备树（DTB）物理内存探测与 Best-Fit 分配器迁移至 `lab4/kern/mm/`。保留 Lab4 原有的页表初始化和中断保护包装。

[验收]
启动输出物理内存范围、空闲页数量以及 check_alloc_page() succeeded!。
````

### 4.2 Lab3 Trap 处理与时钟机制回填

````markdown
[落地执行规范]
1. 在 trapentry.S 中实现 RESTORE_ALL（先恢复 sstatus/sepc/stval，再恢复通用寄存器，最后从槽位 2 恢复 sp）与 __alltraps 入口调用，保留已有的 forkrets。
2. 在 trap.c 中实现 trap_in_kernel、exception_instruction_length、exception_handler 与 interrupt_handler（时钟中断重设并计数）。
3. 验证：完整内核构建通过，`make qemu` 启动且进程调度输出正常。
````
