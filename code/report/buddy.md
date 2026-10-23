# Lab2 Buddy System 实验记录

本文件对应实验任务三:在 ucore 的 `struct pmm_manager` 框架下从零实现 Buddy System 物理页分配器,接入内核,并设计相应测试。任务二的 Best Fit 保持独立,不在本分支修改。

## 一、设计概要

### 数据结构

- `MAX_ORDER = 15`,块大小从 `2^0` 到 `2^15` 页。
- `free_area_t free_area[MAX_ORDER + 1]`,每个 order 一条空闲链表。
- 空闲块首页用 `PageProperty` 标记,`property` 保存 order。
- 已分配块首页使用 `flags` 的私有 bit 2 标记,`property` 保存分配时的 order,供 `free_pages()` 找回整块大小。
- buddy 地址按物理页号计算:`buddy_ppn = ppn ^ (1 << order)`。

### 关键函数

| 函数 | 作用 |
| --- | --- |
| `buddy_init()` | 初始化 16 条 order 链表和总空闲页数 |
| `buddy_init_memmap()` | 清除保留标志,把任意起始/长度的空闲区拆成对齐的 `2^k` 块 |
| `buddy_alloc_pages(n)` | 向上取整到 `2^k`,找最小可用 order,逐级对半拆分 |
| `buddy_free_pages(base, n)` | 按保存的 order 归还整块,并沿 buddy 逐级合并 |
| `buddy_nr_free_pages()` | 返回总空闲页数 |
| `buddy_check()` | 校验链表、对齐、重叠、计数、相邻 buddy 合并,并做基本分配/释放测试 |

### 接入方式

`pmm.c` 增加条件选择:

```c
#ifdef USE_BUDDY_PMM
    pmm_manager = &buddy_pmm_manager;
#else
    pmm_manager = &default_pmm_manager;
#endif
```

Makefile 增加:

```make
ifeq ($(BUDDY),1)
DEFS += -DUSE_BUDDY_PMM
endif
```

默认构建保持原管理器;Buddy 构建使用:

```sh
make -C lab2 clean
make -C lab2 BUDDY=1
```

> 注意:`make` 不跟踪编译宏变化,切换默认/Buddy 前必须先 `make clean`。

## 二、Prompt 1:生成 Buddy System 模块

```text
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
```

## 三、Prompt 2:生成 Buddy System 测试

```text
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
```

## 四、测试覆盖与结果

### 内核内 `buddy_check()`

QEMU 启动时 `check_alloc_page()` 调用 `pmm_manager->check()`;Buddy 管理器执行:

- 扫描所有 order 链表,检查块首、对齐、范围、`property`、`PageProperty`;
- 用临时 flag 位标记已访问页,检测块重叠;
- 检查没有两个同 order 的 buddy 同时空闲(否则说明漏合并);
- 检查 `free_area[order].nr_free` 与链表块大小一致,`nr_free` 与总和一致;
- 分配 1+1 页、5 页(向上取整到 8)、3 页(向上取整到 4),分别释放;
- 检查 `n=0`、`n>nr_free` 的失败路径。

### 主机回归 `test_buddy_host.c`

- 256 页连续内存,初始化为一个 order-8 块;
- 分配 1 页两次,验证第二次返回 buddy 页;释放第一页时 buddy 仍分配,不能合并;
  释放第二页后恢复;
- 5 页请求占 8 页块,3 页请求占 4 页块,验证 `nr_free` 按块大小变化;
- 整块分配 256 页后再次分配失败,释放后恢复;
- 2000 次固定种子随机分配/释放,用占用位图逐步断言 `nr_free`;
- 结束时归还全部页并调用 `check()`。

### 实际结果

| 项目 | 结果 | 证据 |
| --- | --- | --- |
| 默认构建(原管理器) | 退出码 0 | - |
| Buddy 构建 `make BUDDY=1` | 退出码 0 | [buddy-build.log](logs/buddy-build.log) |
| Buddy QEMU 128 MiB | `memory management: buddy_pmm_manager`、`check_alloc_page() succeeded!` | [buddy-qemu.log](logs/buddy-qemu.log) |
| Buddy QEMU 64 MiB | 16384 个描述符、15705 空闲页、自检通过 | [buddy-qemu-64m.log](logs/buddy-qemu-64m.log) |
| 主机回归 | `Buddy host test: PASS (2000 operations)` | [buddy-host.log](logs/buddy-host.log) |

QEMU 关键输出:

```text
memory management: buddy_pmm_manager
physical memory: [0x0000000080000000, 0x0000000088000000)
page descriptors: 32768, npage: 0x88000
free physical memory: [0x0000000080347000, 0x0000000088000000), 31929 pages
check_alloc_page() succeeded!
```

## 五、边界与限制

- Buddy 按 `2^k` 分配,`alloc_pages(5)` 会占 8 页,内部碎片是算法本身的一部分;
- `free_pages(base, n)` 使用分配时保存的 order 归还整块,`n` 只用于校验不超过块大小;
- `MAX_ORDER=15`,单次分配最多 32768 页;超过返回 `NULL`;
- 默认构建不定义 `USE_BUDDY_PMM`,仍使用原管理器;Buddy 只通过 `BUDDY=1` 启用;
- 本机 QEMU 7.0.0 的 `-device loader` 不会填 fw_dynamic 的 `next_addr`,启动验证使用
  `-kernel`;仓库命令未因此修改。

## 六、复现命令

```sh
# 默认构建
make -C lab2 clean
make -C lab2

# Buddy 构建
make -C lab2 clean
make -C lab2 BUDDY=1

# 本机启动(绕过 -device loader 的 fw_dynamic 问题)
timeout -s KILL 12 qemu-system-riscv64 -machine virt -nographic \
    -bios default -kernel lab2/bin/ucore.img

# 主机回归
gcc -std=gnu99 -O1 -g -Wall -Wno-unused -fno-builtin \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I lab2/libs -I lab2/kern/mm -I lab2/kern/debug \
    lab2/tools/test_buddy_host.c lab2/kern/mm/buddy_pmm.c \
    -o /tmp/test_buddy_host
ASAN_OPTIONS=detect_leaks=0 /tmp/test_buddy_host
```
