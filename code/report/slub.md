# Lab2 SLUB 扩展报告与问答

开发分支：`Dev/Lab2/hews/slub`，基于 `Dev/Lab2/main` 的 `7074bb6`。本次只完成 SLUB 扩展，通过公共物理页接口接入，当前底层仍为 First Fit。

## 第一份 prompt：实现模块

下面是根据本次任务设计的实现提示词。它是课程要求的可复用设计材料，不是另一段用户原文或另一次独立模型调用的记录；本次在同一 Codex 会话中按实现、测试两个阶段执行。

```text
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
```

## 第二份 prompt：针对生成模块设计测试

实现阶段已提供 `slub_init()`、`kmalloc()`、`kfree()`，采用八种大小缓存、partial 链表与空 slab 即时归还。针对该公开行为使用下面的测试提示词；测试不能复制内部选槽逻辑来计算答案。

```text
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
```

## 实现问答

### 1. 为什么 SLUB 放在页分配器上面？

页分配器每次至少提供 4096 字节。若每个几十字节的对象独占一页，会浪费大量空间。对象层从页分配器取得页面，将一页分成许多同规格槽位，向调用者返回其中一块可写空间：

```text
kmalloc(40)
→ 选择 64 字节缓存
→ 从 partial slab 的对象链弹出一个槽位
→ 缓存没有可用 slab 时，先向 alloc_pages(1) 申请一页

kfree(ptr)
→ 归还对象槽位
→ 最后一个对象归还时，free_pages(page, 1)
```

源码见 [slub.h](../kern/mm/slub.h)、[slub.c](../kern/mm/slub.c)；接入点见 [init.c](../kern/init/init.c)。底层仍由 `pmm_manager` 决定，不需要 SLUB 重新实现物理页算法。

### 2. 页内布局和对象链是什么？

当前 64 位构建中，slab 结构体为 120 字节，对齐后的对象起点偏移为 128。它包含所属页、缓存、页数、使用数量、总槽位数、空闲对象链头、partial／所有权链节点和占用位图：

```text
一页 4096 字节
┌──────────────┬────────┬────────┬────────┬─────────┐
│ 元数据及对齐 │ 对象 0 │ 对象 1 │ 对象 2 │ ……      │
│ 128 字节     │        │        │        │         │
└──────────────┴────────┴────────┴────────┴─────────┘
```

对象空闲时，其前一个指针大小的空间保存下一空闲对象地址；对象分配后，这些字节归调用者使用。无需为每个对象再调用一次内存分配器来创建链表节点。

| 对象规格（字节） | 一页槽位数 `floor((4096-128)/规格)` |
| --- | --- |
| 16 / 32 / 64 / 128 | 248 / 124 / 62 / 31 |
| 256 / 512 / 1024 / 2048 | 15 / 7 / 3 / 1 |

### 3. partial、full 和 empty 怎样转换？

新 slab 初始化对象链并加入缓存的 partial 链表。每次申请弹出一个对象并增加 `inuse`；填满后移出 partial，但仍在所有权登记链表中。满 slab 释放一个对象后重新加入 partial。最后一个对象归还时，移出两张链表并立即归还物理页。

当前没有长期保留空 slab；这样全部对象释放后可以检查底层空闲页总数恢复。对象对齐保证是 16 字节，没有承诺与 Linux 完全相同的每种尺寸对齐规则。

### 4. 大于 2048 字节的任意长度怎么处理？

直接用连续页容纳“头部＋请求长度”，以向上取整的页数申请，并在头部记录传给底层的页数。比如 6000 字节需要 `ceil((128+6000)/4096)=2` 页；释放时不要求调用者再次提供长度。

大对象路径不一定至少占两页：2049 字节加头部仍能放进一页。反过来，4096 字节加头部需要两页。这是对象接口的元数据成本；如果调用者本来就需要原始整页，可以直接使用页接口。

### 5. 为什么 kfree 不需要长度，还能拒绝部分非法调用？

返回指针位于所属 slab 或大对象跨度的第一页。实现先把指针数值向下按页对齐，再查自己维护的所有权登记链表；找到之前不读取这个外来地址的内容。

小对象再检查是否为合法槽位起点、占用位是否为真；大对象要求等于头部后面的精确返回地址。槽位内偏移、陌生指针和可检测的重复释放通过 panic 拒绝。地址若已重新分配给新对象，则旧指针与新指针数值相同，当前没有代际标识来区分它们。

### 6. “任意大小”和失败处理怎样界定？

任意正长度都能作为请求，不要求调用者将长度先取整到规格；成功与否取决于可用内存和连续页条件。实际槽位可能大于请求，这是内部碎片。请求零字节、加头部会溢出或页面不足时返回 NULL；`kfree(NULL)` 无动作。返回内容不保证清零。

当物理页耗尽时，已有 slab 还有空闲槽位就仍可满足相应请求；需要新页的请求才会失败。失败不能改变存活对象或遗失已持有页。

### 7. 本实现保留了 SLUB 的哪些思想，省略了什么？

保留页／对象两层、按大小缓存、对象内 freelist、partial slab 复用及空 slab 回收。参考 Linux 官方 [内存分配指南](https://docs.kernel.org/core-api/memory-allocation.html) 与 [v6.12 SLUB 源码的结构说明](https://raw.githubusercontent.com/torvalds/linux/v6.12/mm/slub.c)，按 ucore 接口重新实现，没有复制 Linux 代码。

当前面向单线程、无并发调用的实验场景，未实现 per-CPU、NUMA、锁、GFP、构造析构或 `krealloc`。缓存选择最多比较 8 档；从已有 partial slab 取槽位为常数次操作；新 slab 需初始化所有槽位；`kfree` 的所有权查找为 `O(当前持有跨度数)`。因此不宣称具有 Linux SLUB 的完整性能和并发能力。

## 测试覆盖与有效性

内核测试见 [slub_test.c](../kern/mm/slub_test.c)，主机适配与入口见 [test_slub_host.c](../tools/test_slub_host.c)、[run_slub_host.py](../tools/run_slub_host.py)。测试维护独立的存活对象表、请求长度和数据模式，不读取私有 slab 布局。

| 测试 | 核心行为／边界 | 为什么能发现错误 |
| --- | --- | --- |
| 29 种尺寸同时持有 | 各缓存档边界、2048/2049、4095/4096/4097、8193 | 按请求长度写满并比对，结合地址区间检查发现容量不足、越界覆盖或错误取整 |
| 对齐和区间比较 | 所有成功对象 | 按公开的 16 字节约定核对地址，逐对确认存活范围不重叠 |
| 填满多个 slab | 根据实际物理页消耗识别两个满 slab | 不硬编码头部大小，能发现“一对象一页”和没有复用 partial 的错误 |
| full → partial → full | 释放唯一可用槽再申请 | 返回该槽且不消耗新页，验证满 slab 的重新登记 |
| 交错释放及空 slab 归还 | 偶数／奇数次序释放、大小混合归还 | 页计数回到起点，检查回收与链表状态转换 |
| 极值 | 0、NULL、`SIZE_MAX`、`SIZE_MAX-PGSIZE` | 区分加法溢出与总容量不足，确保失败不耗页 |
| 真实耗尽底层页 | 小对象保留、缓存内空槽、新缓存与大对象请求 | 无物理页时旧槽仍可申请，新页请求失败，原对象数据保持；释放后重新申请成功 |
| 固定种子混合操作 | 内核 2000 次；主机额外 20000 次 | 独立检查存活区间和内容，扩展尺寸、释放次序与状态组合 |
| 7 类非法释放 | 陌生指针、小对象内部、大对象头页内部与后续页、三种重复释放 | 独立进程必须在对应检查处 panic，不能以 sanitizer 崩溃代替正常拒绝 |
| 主机 PMM 定向回归 | 递增两段登记、递增释放、一次申请完整 256 页 | 覆盖尾插缺陷；既检查总量，也检查连续区间恢复 |

ASan/UBSan 检查主机实际执行中可检测的地址错误和未定义行为。模拟 RAM 是一整块主机数组，sanitizer 不自动知道每个自定义对象的边界，因此对象间覆盖主要由独立区间、模式数据和状态断言发现。有限固定序列不是所有输入的证明，也没有验证尚未集成的 Buddy 实现。

## 实际调试与修复

第一轮主机构建额外使用 `-Wextra`，触发原 First Fit 自检的有符号比较告警；后改为与内核一致的 `-Wall -Werror`，保留原测试函数，见 [初始构建记录](logs/slub-host-initial.log)。

随后随机回归在第 32 步检测到 4097 字节对象与存活的 7 字节对象重叠，见 [失败记录](logs/slub-host-overlap.log)。同轮 QEMU 未完成耗尽回收测试，见 [128 MiB 修复前记录](logs/slub-qemu-128m-before-pmm-fix.log) 与 [64 MiB 修复前记录](logs/slub-qemu-64m-before-pmm-fix.log)。GDB 检查到 PMM 总计数已恢复，但空闲链表存在无效尾节点。

根因是 `default_init_memmap()` 和 `default_free_pages()` 在尾部 `list_add()` 后没有 `break`，遍历会再次处理刚插入的节点并将它插到自身之后。修复只在两个尾插分支各增加一个 `break`，见 [default_pmm.c](../kern/mm/default_pmm.c)。原 `basic_check()`、`default_check()`、Best Fit 文件与评分工具保持不变；定向回归和同一随机序列复验后通过。

## 验证命令与结果

在仓库根目录执行：

```bash
source ../Lab1前置准备/activate.sh
make -C lab2 clean
make -C lab2
timeout 15s make -C lab2 qemu
timeout 15s make -C lab2 qemu QEMU='qemu-system-riscv64 -m 64M'
make -C lab2 slub-host-test
```

| 已执行项目 | 结果与证据 |
| --- | --- |
| 干净构建 | 退出 0，见 [slub-build.log](logs/slub-build.log) |
| 128 MiB QEMU | 页自检与 SLUB 自检成功，空闲页恢复 31927，见 [slub-qemu-128m.log](logs/slub-qemu-128m.log) |
| 64 MiB QEMU | 页自检与 SLUB 自检成功，空闲页恢复 15703，见 [slub-qemu-64m.log](logs/slub-qemu-64m.log) |
| 主机 sanitizer 回归 | 公共自检 2000 次＋额外 20000 次、7 类预期 panic、256 页整体恢复均通过，见 [slub-host.log](logs/slub-host.log) |
| 清理后的 `make test` | 原 First Fit 自检及 SLUB 自检通过，见 [slub-test.log](logs/slub-test.log)；First Fit 没有 Best Fit 的六阶段计分输出 |
| 规格与仓库规范 | SLUB change 严格校验通过；治理内置自测、分支／范围／Prompt 门禁通过，见 [slub-governance.log](logs/slub-governance.log)；`git diff --check` 通过 |
| 中间提交可构建性 | 修复、实现、测试三组暂存快照分别导出到隔离目录构建，均退出 0，见 [slub-commit-builds.log](logs/slub-commit-builds.log)；文档组的代码与测试组一致，复用该构建结果 |
| 暂存快照运行复验 | 测试组快照再次完成主机 sanitizer／非法释放检查，以及 128/64 MiB 启动自检，见 [slub-staged-checks.log](logs/slub-staged-checks.log) |

QEMU 最终由外部限时结束，退出码 124；通过依据是完整成功标记及无 panic。新模块和测试增大了内核，当前 `end` 为 `0xffffffffc0208e0c`，描述符起点变为 `0x80209000`；这解释了空闲页比之前页初始化阶段少 2 页。128/64 MiB 空闲起点分别为 `0x80349000`、`0x802a9000`，不是固定写入代码的常量。

当前仍为 First Fit，原评分的 Best Fit 管理器检查不满足，不把本扩展宣称为整个 Lab2 的 `20/20`。最终小组集成后需要复验实际选择的页分配器。日志不代替最终提交要求的真实运行截图。

## 提交分组与交接

2026-10-22 用户明确授权测试通过后 commit 并 push。提交按以下职责拆分并保留顺序：

1. `fix(lab2): stop first-fit scans after appending free blocks`：两个 `break`；对应 PMM 定向回归随第 3 组测试交付。
2. `feat(lab2): add a page-backed SLUB object allocator`：对象层、启动初始化与对应行为规格、实现提示词。
3. `test(lab2): verify SLUB reuse, exhaustion and invalid frees`：内核／主机检查及测试提示词；启动自检调用随测试提交加入。
4. `docs(lab2): record SLUB verification and defense notes`：真实日志、报告、review 与答辩材料。

提交时只暂存对应阶段路径／代码块，并在隔离目录验证每个暂存快照可编译。提交顺序可用 `git log --reverse --oneline 7074bb6..Dev/Lab2/hews/slub` 核对，推送目标为 `origin/Dev/Lab2/hews/slub`。PR 目标为 `Dev/Lab2/main`，最终选择 Rebase and merge；Best Fit 与 Buddy 的组员开发内容不混入本分支。
