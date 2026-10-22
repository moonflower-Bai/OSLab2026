# Lab2 测试报告

本文件记录 Lab2 物理内存与 Best Fit 分配器集成后的测试内容、测试方法和实际结果。测试对象为 `Dev/Lab2/cyx/free-merge` 分支，重点覆盖释放合并（`best_fit_init_memmap()`、`best_fit_insert_block()`、`best_fit_free_pages()`）以及它与页初始化、选块/拆分的整体配合。

## 一、测试对象

| 项目 | 内容 |
| --- | --- |
| 仓库 | `OSLab2026` |
| 分支 | `Dev/Lab2/cyx/free-merge` |
| 测试时 HEAD | `034d803`（合并三位组员的工作分支） |
| 工作区状态 | 测试时包含一份 66 行的未提交去重修复：删除重复的 `best_fit_insert_block` / `best_fit_init_memmap` / `best_fit_free_pages` |
| 被测模块 | `lab2/kern/init/entry.S`、`lab2/kern/mm/pmm.c`、`lab2/kern/mm/best_fit_pmm.c` |
| 分工 | hews 负责启动映射与页初始化；baijingren 负责 Best Fit 选块与拆分；cyx 负责释放合并与测试 |

> 测试执行时 HEAD 为 `034d803`，工作区的去重修复尚未提交，因此 P2 日志头记录的仍是 `034d803`。复现时以实际提交后的 HEAD 为准。

## 二、测试环境

| 工具 | 实际版本 |
| --- | --- |
| 操作系统 | WSL2，Linux 5.15.167.4-microsoft-standard-WSL2 |
| RISC-V GCC | 16.1.0（`riscv64-unknown-elf-gcc`） |
| RISC-V Binutils | 2.47.20260726 |
| QEMU | 7.0.0（自编译） |
| OpenSBI | v1.0（QEMU 自带的 fw_dynamic） |
| GNU Make | 4.2.1 |
| 宿主编译器 | gcc 9.4.0（仅用于主机位图回归） |
| Python | 3.13.5 |

本机 QEMU 环境有一个已知限制：仓库的 `Makefile` / `tools/grade.sh` 使用 `-device loader,file=...,addr=0x80200000`，而 QEMU 7.0.0 的 `-bios default` 加载 fw_dynamic；QEMU 只在 `-kernel` 路径填充 `fw_dynamic_info.next_addr`，`-device loader` 不填充，导致固件跳到地址 0。测试时因此采用两种本地绕过方式：

- QEMU 启动测试：使用 `-kernel lab2/bin/ucore.img`；
- `make grade` / P2 测试：在 PATH 前放一个包装层，把 `-device loader` 参数转成 `-kernel`。

仓库本身没有为此修改，`-device loader` 保持原样；本机 QEMU 环境的正式修复（构建 OpenSBI fw_jump 并让 `-bios default` 使用它）另行处理。

128 MiB 与 64 MiB 启动时，DTB 地址分别为 `0x87000000` 和 `0x83000000`，与第二节报告中 hews 阶段使用的 QEMU 包装层不同，因此测试结果单独记录。

## 三、测试内容与方法

### 3.1 构建测试

**目的**：确认合并后的源码能完整编译、链接，没有重复定义、缺符号或类型冲突。

**方法**：

```sh
make -C lab2 clean
make -C lab2
```

**判定**：退出码为 0，并生成 `lab2/bin/kernel`、`lab2/bin/ucore.img`。

### 3.2 内核启动与框架自检

**目的**：验证从 OpenSBI 交接、DTB 解析、`page_init()` 建页描述符，到 `best_fit_check()` 自检的完整启动路径。

**方法**：

```sh
# 128 MiB
timeout -s KILL 12 qemu-system-riscv64 -machine virt -nographic \
    -bios default -kernel lab2/bin/ucore.img

# 64 MiB
timeout -s KILL 12 qemu-system-riscv64 -machine virt -m 64M -nographic \
    -bios default -kernel lab2/bin/ucore.img
```

内核启动后依次执行：

```text
dtb_init() → cons_init() → pmm_init()
    ├─ init_pmm_manager()   选择 best_fit_pmm_manager
    ├─ page_init()          读取 DTB、建立 pages[]、把空闲区交给 init_memmap()
    └─ check_alloc_page()   → pmm_manager->check() = best_fit_check()
```

`best_fit_check()` 是框架自带测试，内容包括：

- 遍历空闲链表，断言每个节点都是空闲块首页，且所有 `property` 之和等于 `nr_free_pages()`；
- `basic_check()`：分配 3 页、释放 3 页、再分配，检查计数和链表恢复；
- 具体合并场景：分配 5 页 `p0..p0+4`，释放 `p0+1` 起 2 页和 `p0+4` 起 1 页，断言 `alloc_pages(4)` 失败、`p0[1].property == 2`；再分配 1 页应得到 `p0+4`，分配 2 页应得到 `p0+1`；最后 `free_pages(p0, 5)` 必须合并成一个 5 页块，并能再次整块分配。

**判定**：串口输出包含 `memory management: best_fit_pmm_manager` 和 `check_alloc_page() succeeded!`，且没有 `kernel panic` / `assertion failed`。QEMU 在内核 `while (1)` 处不会自行退出，由 `timeout` 终止，退出码 137 属于预期。

### 3.3 原始评分测试

**目的**：运行课程提供的注入式测试，验证页初始化和 Best Fit 管理器接线。

**方法**：

```sh
make -C lab2 grade
```

`tools/inject.py` 会：

1. 把 `tools/test_lab2.c/.h` 复制到临时副本的 `kern/test/`；
2. 在副本 `Makefile` 的 `KINCLUDE` / `KSRCDIR` 加入 `kern/test`；
3. 在 `pmm.c` 中插入 `#include <test_lab2.h>`；
4. 在 `pmm_init()` 末尾注入 `test_page_init(); test_best_fit();`。

`test_page_init()` 检查 8 项：

1. `va_pa_offset == PHYSICAL_MEMORY_OFFSET`；
2. DTB 报告的物理内存起点/大小非 0；
3. `npage == PPN(mem_end)` 且 `npage > nbase`；
4. `pages` 非空、4 KiB 对齐、位于内核 `end` 之后；
5. `0 < nr_free_pages() <= npage - nbase`；
6. 内核镜像末尾页仍是 `PageReserved`；
7. 页描述符数组本身仍是 `PageReserved`；
8. `pa2page(page2pa(q)) == q` 往返正确。

`test_best_fit()` 检查 `pmm_manager == &best_fit_pmm_manager`。

**判定**：串口输出同时包含

```text
[Test 1] Page Init: PASS
[Test 2] Best-Fit: PASS
```

每项 10 分，总分 20 分。

### 3.4 P2 黑盒测试

**目的**：在与项目代码隔离的副本中，用公开接口验证 Best Fit 的不变量、选块、合并、标志残留和失败路径。

**方法**：

```sh
sh test/lab2/run_p2.sh
```

测试套件把 `lab2/` 复制到临时目录，只在副本中注入测试、构建并启动 QEMU；测试代码只使用 `alloc_pages`、`free_pages`、`nr_free_pages` 和 `pmm_manager`，不引用 `free_list`、`nr_free` 或分配器的 static 函数。

隔离方式：

- 启动时用 `alloc_pages` 把全部空闲页抽干；
- 在抽出的连续段里划出 64 页试验区，两端各留 1 页已分配页作为 guard；
- 每个用例开始时试验区全部已分配、`nr_free_pages() == 0`，结束时恢复；
- 全部用例跑完归还所有页，并断言回到运行前的空闲页基线；
- 断言失败时打印用例名、行号和试验区页状态，然后 panic，不尝试修复坏状态。

覆盖范围：

| 组 | 内容 | 用例数 |
| --- | --- | --- |
| D0 | 管理器确实是 `best_fit_pmm_manager` | 1 |
| D1 | 不变量巡检器 B1–B5 及 5 种损坏状态自检 | 1 |
| D2 | 最小块选择、并列、切分边界（含 AT-P2-01/02/03/04/13） | 11 |
| D3 | 释放合并组合（AT-P2-05/06/07/08 及合并链） | 5 |
| D4 | 标志、引用计数、地址顺序（含 AT-P2-09/10/14） | 5 |
| D5 | 三个固定种子各 1500 次随机模型验证 | 3 |
| D6 | 失败分配无副作用（含 AT-P2-11/12） | 4 |

D1 的 B1–B5 分别是：

- B1：链表节点与 `PG_property` 首页一一对应、按地址严格升序、前后指针一致；
- B2：首页 `0 < property <= 剩余页数`；块内页无 `PG_property`、无 `PG_reserved`、`ref == 0`；
- B3：所有块 `property` 之和等于 `nr_free_pages()`；
- B4：块之间不重叠；
- B5：地址相邻的空闲块必须已经合并。

D3 的五个用例直接对应释放合并：只与后继合并、只与前驱合并、前后同时合并、中间隔着已分配页时不合并、连续合并后再整块分配。D4 检查被吸收块首的 `PG_property` 是否清除、切分和乱序合并后的 flags、`free_pages` 是否清零 `ref`、插入是否保持地址顺序、`init_memmap` 乱序登记是否自动合并。

**判定**：每个用例输出 `[P2] PASS ...`，最后汇总 `SUMMARY run=30 pass=30 fail=0` 和 `RESULT: PASS`。

### 3.5 主机位图回归

**目的**：在主机上用独立位图模型验证真实 `best_fit_pmm.c` 的选块、释放和合并，作为 RISC-V/QEMU 测试的补充。

**方法**：使用参考包的 `tools/test_best_fit_host.c`，在 `/tmp` 作为临时脚手架编译当前 `best_fit_pmm.c`：

```sh
gcc -std=gnu99 -O1 -g -Wall -Wno-unused -fno-builtin \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I lab2/libs -I lab2/kern/mm -I lab2/kern/debug \
    test_best_fit_host.c lab2/kern/mm/best_fit_pmm.c -o /tmp/test_merged
ASAN_OPTIONS=detect_leaks=0 /tmp/test_merged
```

测试内容：

- 256 个 `struct Page` 加桩函数模拟内核环境；
- 先乱序登记两个相邻区域（`pages+128` 起 128 页、`pages` 起 128 页），要求合并为一个 256 页块；
- 再做 10,000 次随机分配/释放，测试侧用 `occupied[]` 位图独立计算“最小可容纳块”的起点，断言分配器返回的正是该页；
- 每一步断言 `nr_free_pages()` 等于位图空闲页数；
- 结束时释放全部页，断言恢复成一个 256 页块。

**判定**：输出 `Host Best Fit: PASS (10000 operations, seed 0x4c414232)`，且 ASan/UBSan 没有报告。

> 该测试脚本来自参考包，没有复制进仓库；P2 的 D5 已覆盖同类随机模型，避免重复维护两份等价测试。

## 四、测试结果

### 4.1 汇总

| 测试 | 结果 | 证据 |
| --- | --- | --- |
| 构建 | 退出码 0 | [best-fit-build.log](logs/best-fit-build.log) |
| 128 MiB 启动 | `best_fit_pmm_manager`、31929 个空闲页、`check_alloc_page() succeeded!` | [best-fit-qemu.log](logs/best-fit-qemu.log) |
| 64 MiB 启动 | 16384 个描述符、15705 个空闲页、自检通过 | [best-fit-qemu-64m.log](logs/best-fit-qemu-64m.log) |
| 原始评分 | **20/20** | [best-fit-grade.log](logs/best-fit-grade.log) |
| P2 黑盒 | **30/30 PASS** | [p2_blackbox_20261022-000744.log](../../test/lab2/results/p2_blackbox_20261022-000744.log) |
| 主机位图回归 | **10,000 次操作 PASS** | [best-fit-host.log](logs/best-fit-host.log) |

### 4.2 启动输出

128 MiB：

```text
memory management: best_fit_pmm_manager
physical memory: [0x0000000080000000, 0x0000000088000000)
page descriptors: 32768, npage: 0x88000
free physical memory: [0x0000000080347000, 0x0000000088000000), 31929 pages
check_alloc_page() succeeded!
satp virtual address: 0xffffffffc0205000
satp physical address: 0x0000000080205000
```

64 MiB：

```text
DTB Address: 0x83000000
Size: 0x0000000004000000 (64 MB)
End:  0x0000000083ffffff
memory management: best_fit_pmm_manager
physical memory: [0x0000000080000000, 0x0000000084000000)
page descriptors: 16384, npage: 0x84000
free physical memory: [0x00000000802a7000, 0x0000000084000000), 15705 pages
check_alloc_page() succeeded!
```

### 4.3 原始评分

```text
-check page init:                          OK
-check best fit:                           OK
Total Score: 20/20
```

### 4.4 P2 黑盒

```text
[P2] SUMMARY run=30 pass=30 fail=0
RESULT: PASS
```

分组结果：D0 1/1、D1 1/1、D2 11/11、D3 5/5、D4 5/5、D5 3/3、D6 4/4。D1 自检中，巡检器分别捕获了 B5、B2/B4、B3、B3、B2 五种故意破坏的状态。

### 4.5 主机位图回归

```text
Host Best Fit: PASS (10000 operations, seed 0x4c414232)
```

## 五、覆盖分析

| 被测函数 | 主要覆盖测试 |
| --- | --- |
| `page_init()` | 原始评分 `test_page_init()`；128 MiB / 64 MiB 启动 |
| `best_fit_init()` | 启动路径；P2 D0 |
| `best_fit_init_memmap()` | 启动登记；P2 D4 乱序登记；主机回归乱序登记 |
| `best_fit_insert_block()` | P2 D1（B1/B5）、D3、D4 地址顺序、D5 |
| `best_fit_alloc_pages()` | P2 D2、D5、D6；框架 `best_fit_check()` |
| `best_fit_free_pages()` | 框架 `best_fit_check()` 的释放场景；P2 D3、D4、D5、D6 |
| `best_fit_nr_free_pages()` | P2 D1 B3、D5；框架计数检查 |
| 管理器接线 | 原始评分 `test_best_fit()`；P2 D0 |

释放合并的三个函数在测试中分别对应：

- `init_memmap`：保留页清标志、整段登记、乱序登记时与前/后块合并；
- `insert_block`：按地址插入、前驱合并、后继合并、`nr_free` 只在插入时增加一次；
- `free_pages`：释放前断言非保留且非空闲块首、逐页清 flags/property/ref、复用插入与合并逻辑。

## 六、问题与限制

### 6.1 合并后的重复定义

合并三位组员的工作分支后，`best_fit_pmm.c` 同时存在两套 `best_fit_insert_block`、`best_fit_init_memmap`、`best_fit_free_pages`：

- 队友分支已包含一套完整 Best Fit 实现（包括本应由 cyx 负责的释放合并）；
- cyx 的提交又在文件末尾加入自己的实现；
- Git 的文本合并未报冲突，编译器报 `conflicting types` / `redefinition`。

处理方式是删除队友版本的三个重复函数，保留 cyx 的版本；队友的 `best_fit_init`、`best_fit_alloc_pages`、`best_fit_nr_free_pages`、管理器表和框架测试保持原样。修复后构建、启动和全部测试通过。

### 6.2 QEMU 环境

本机 QEMU 7.0.0 的 `-bios default` 使用 fw_dynamic，`-device loader` 不会填充 `next_addr`，因此仓库原生的 `make qemu` 在本机不会进入内核。测试使用 `-kernel` 或本地 PATH 包装层完成，仓库命令没有修改。本机 QEMU 环境待后续修复（构建 OpenSBI fw_jump 并让 `-bios default` 使用它）。

### 6.3 测试本身的边界

- 原始评分只有两个 PASS 检查，对释放合并的直接覆盖不深；深度主要来自启动时 `check_alloc_page()` 执行的 `best_fit_check()` 和 P2 套件；
- P2 是黑盒模型测试，并列最小块允许任选，不与 First Fit 锁步比较；
- 主机回归使用 256 页的简化模型和桩函数，不能替代 RISC-V/QEMU 测试；
- 测试日志记录的是包含未提交去重修复的工作区，P2 日志头的 HEAD 为 `034d803`。

## 七、复现命令

```sh
# 1. 构建
make -C lab2 clean
make -C lab2

# 2. 启动（本机用 -kernel 绕过 fw_dynamic 的 loader 问题）
timeout -s KILL 12 qemu-system-riscv64 -machine virt -nographic \
    -bios default -kernel lab2/bin/ucore.img

timeout -s KILL 12 qemu-system-riscv64 -machine virt -m 64M -nographic \
    -bios default -kernel lab2/bin/ucore.img

# 3. 原始评分（本机需要把 -device loader 转成 -kernel 的 PATH 包装层）
make -C lab2 grade

# 4. P2 黑盒
QEMU=/home/oisdoaiu/decode/qemu-7.0.0/build/qemu-system-riscv64 \
    sh test/lab2/run_p2.sh

# 5. 主机位图回归（参考包脚本，按第 3.5 节的 gcc 命令编译）
```

## 八、结论

在合并后的 Lab2 代码上，构建、128 MiB / 64 MiB 启动、原始评分（20/20）、P2 黑盒（30/30）和主机位图回归（10,000 次）全部通过。释放合并的三种路径——只前并、只后并、两侧并——以及不合并、乱序登记、失败分配无副作用等边界均有覆盖。当前唯一未闭环的是本机 QEMU 的 `-device loader` 启动环境，该问题不影响代码本身的验证结论。
