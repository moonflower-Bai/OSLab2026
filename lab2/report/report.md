# Lab2：物理内存和页表实验报告

本文件是 hews 的启动映射与物理页初始化阶段报告；Best Fit 和最终集成由其他组员后续补充。详细讲解见 [代码 review](review.md)，分工见 [三人分工](division.md)。

## 实验基本信息

| 项目 | 内容 |
|---|---|
| 实验名称 | Lab2：物理内存和页表 |
| 小组成员 | 待小组填写 |
| 完成日期 | 待最终验收 |
| 原始源码基线 | `Dev/Lab2/main`，`d52989b` |
| 当前变基起点 | `Dev/Lab2/main`，`e83e10e`；实验源码与原基线一致 |
| 开发分支 | `Dev/Lab2/hews/page-init` |

### 小组分工

hews 负责启动映射、物理页初始化与相关环境；baijingren 负责 Best Fit 选块和拆分；第三位成员负责释放合并及测试。完整模块与报告安排见 [分工文档](division.md)，姓名、学号及实际贡献待小组按事实填写。

## 一、实验目的

1. 理解 RISC-V 启动时的 Sv39 映射、虚拟地址与物理地址的关系。
2. 依据设备树提供的内存范围建立物理页描述信息，区分已占用页与可分配页。
3. 实现连续物理页的 Best Fit 分配与回收，并理解拆分、碎片和相邻块合并。
4. 使用真实编译、运行和评分结果验证实现，并记录 AI 协作中的实际迭代。

## 二、实验环境

| 工具 | 本机实际验证版本 |
|---|---|
| RISC-V GCC | 14.2.0 |
| RISC-V Binutils | 2.45.50.20251209 |
| QEMU | 10.2.1 |
| GDB | 17.1（gdb-multiarch 兼容入口） |
| GNU Make | 4.4.1 |
| Python | 3.14.4 |
| bc | 1.07.1 |
| OpenSpec | 1.14.0 |
| AI 工具 | Codex；底层模型精确版本待工具配置核实 |

QEMU、GDB、OpenSpec 复用仓库外的 Lab1 前置准备工具。QEMU 包装层在 loader 启动方式下选择 OpenSBI FW_JUMP；实际启动已验证 DTB 地址为 `0x82200000`，默认内存范围为 `[0x80000000, 0x88000000)`。

## 三、实验整体逻辑分析

启动固件交接 hart ID 和 DTB 地址，入口汇编启用已有 Sv39 映射并进入内核。内核解析 DTB，再初始化物理内存管理器，建立页描述符和空闲块，最后运行分配器自检。

实施顺序为：核对基线与环境 → 初始化物理页并用现有 First Fit 验证 → 实现并启用 Best Fit → 完成评分与报告。每一阶段单独记录结果。

## 四、实验内容与实现

### 基线核对

2026-10-20 获取的 `Dev/Lab2/main` 基线包含 39 个骨架文件，与仓库外原始 `lab2_new` 逐文件字节一致。基线编译退出码为 2，首先报出 Best Fit 的 `free_list`、`nr_free` 未声明，属于待实现部分，见 [基线编译日志](logs/baseline-build.log)。

代码任务依据骨架的两处标记和提供的测试确认。指导书练习页面本次访问超时，实验要求页面返回 HTTP 502，因此书面练习和 Challenge 的完整清单尚待核对。

### 物理内存初始化

涉及函数为 `kern/mm/pmm.c` 中的 `static void page_init(void)`，本阶段由 hews 负责；启动所需修复位于 `kern/init/entry.S`。

初始化先设置 `va_pa_offset = PHYSICAL_MEMORY_OFFSET`，读取 DTB 的内存起点和大小。当前页号转换以 `DRAM_BASE` 为基点，初始映射覆盖从该地址开始的 1 GiB，因此对起点和范围作断言，防止把不可映射的页纳入管理。

`npage` 表示内存末端的物理页号，描述符数量是 `npage - nbase`，而不是 `npage`。`pages[]` 在内核 `end` 后按 4 KiB 对齐，所有描述符先标记保留，再将描述符数组之后的完整页交给 `init_memmap()`。

默认 128 MiB 启动中的实际布局为：

| 区域或参数 | 本阶段实际值 |
|---|---|
| 内存范围 | `[0x80000000, 0x88000000)` |
| `nbase` / `npage` | `0x80000` / `0x88000` |
| 描述符数量 | 32768 |
| `sizeof(struct Page)` | 40 字节 |
| 描述符数组物理起点 | `0x80207000` |
| 空闲物理区间 | `[0x80347000, 0x88000000)` |
| 空闲页数 | 31929 |

这里的精确位置来自本阶段编译，后续内核体积变化可能改变数组和空闲区起点。算法按链接符号与实际内存范围计算，不依赖这些示例地址。

#### 启动问题的实际修复

第一次编译成功后，QEMU 停在固件输出。GDB 在物理地址 `0x80200034` 命中写入 `satp` 的指令，单步后 PC 为 `0x80200038`，但该地址已经不能取指，见 [GDB 日志](logs/boot-debug-gdb.log)。原因是原始根页表只有索引 511 的高地址叶项，而启用分页不会自动修改 PC。

在 `entry.S` 为根页表索引 2 增加相同物理 1 GiB 区间的身份映射，使入口能继续执行刷新 TLB、设置高地址栈和跳转内核的指令；索引 511 的高地址映射保留。根表仍为 512 项，未新增运行时页表管理。

修复后构建退出为 0，ELF 为 64 位 RISC-V；原 First Fit 自检输出 `check_alloc_page() succeeded!`。QEMU 在成功输出后继续空转，由外部 `timeout` 终止，退出码 124 不作为单独的通过依据。见 [编译日志](logs/page-init-build.log)、[ELF 信息](logs/page-init-elf.log) 和 [初始化运行日志](logs/page-init-qemu.log)。

### Best Fit 连续物理页分配

由 baijingren 和第三位成员按函数分工完成，参考代码另行打包。合并后的分支已在 `pmm.c` 中将 `pmm_manager` 切换为 `&best_fit_pmm_manager`，选块/拆分与释放合并分别由两位成员实现；算法细节和场景分析由对应成员补充。

### 实际提示词与迭代

本 change 的用户原文证据保存在 `openspec/changes/implement-lab2-physical-memory/prompt.md`。报告只记录实际实现和修复过程，不编造逐模块提示词或迭代次数。

## 五、测试与验证

完整的测试内容、方法和结果见 [测试报告](test-report.md)。以下为摘要。

Best Fit 与释放合并集成后，第三位成员在合并分支上完成以下复现（2026-10-22）。证据日志见下表。

| 项目 | 命令 | 结果 | 证据 |
| --- | --- | --- | --- |
| 构建 | `make -C lab2 clean && make -C lab2` | 退出码 0 | [构建日志](logs/best-fit-build.log) |
| 128 MiB 启动 | `qemu-system-riscv64 ... -kernel lab2/bin/ucore.img` | `best_fit_pmm_manager`；31929 个空闲页；`check_alloc_page() succeeded!` | [启动日志](logs/best-fit-qemu.log) |
| 64 MiB 启动 | 同上，加 `-m 64M` | 16384 个描述符；15705 个空闲页；自检通过 | [64 MiB 日志](logs/best-fit-qemu-64m.log) |
| 原始评分 | `make -C lab2 grade` | 20/20（page init OK、best fit OK） | [评分日志](logs/best-fit-grade.log) |
| P2 黑盒 | `sh test/lab2/run_p2.sh` | 30/30 PASS，含 D3 合并组合与 D5 三个种子各 1500 次随机操作 | [P2 日志](../../test/lab2/results/p2_blackbox_20261022-000744.log) |
| 主机位图回归 | 参考包 `test_best_fit_host.c` 编译当前 `best_fit_pmm.c`，ASan/UBSan | 10,000 次操作 PASS | [主机日志](logs/best-fit-host.log) |

补充说明：

- 本节复现使用本机 QEMU 7.0.0（自编译，OpenSBI v1.0，fw_dynamic）；128 MiB 下 DTB 地址为 `0x87000000`。与第二节 hews 阶段所用包装层的版本和 DTB 地址不同，因此单独记录。
- 仓库的 `make qemu` / `make grade` 仍使用 `-device loader`；本机 QEMU 7.0.0 的 fw_dynamic 不填 `next_addr`，所以启动验证用 `-kernel`，评分用本地 PATH 包装层把 loader 参数转成 `-kernel`。仓库命令没有因此修改，本机 QEMU 环境待修复。
- 合并后曾出现两份释放合并实现，导致 `best_fit_insert_block`、`best_fit_init_memmap`、`best_fit_free_pages` 重复定义；保留第三位成员的版本后构建通过。上述测试基于包含该未提交修复的工作区，日志记录的 HEAD 为 `034d803`。
- P2 套件在 `page_init()` 合入后不再需要 `P2_PAGE_INIT_FROM`，直接运行即 30/30 通过。覆盖范围包括 D1 链表不变量巡检、D2 选块/并列/切分、D3 前并/后并/双侧并/不并、D4 标志与引用计数清理、D5 随机模型、D6 失败分配无副作用。
- 主机位图回归的脚本来自参考包，仅在 `/tmp` 作为临时脚手架编译运行，没有复制进仓库；P2 的 D5 已覆盖同类随机模型，避免重复维护两份等价测试。

## 六、实验总结与收获

待模块实现与验证后补写。

## 七、交付待办

- 核对完整指导书中的书面练习和 Challenge。
- 由小组补齐成员、分工和最终验收日期。
- 完成真实测试截图和报告链接。
- 最终提交需映射为根级 `code/`、`report/`；`report/prompt.md` 与当前提示词门禁的兼容方式待交付整理阶段审阅。
