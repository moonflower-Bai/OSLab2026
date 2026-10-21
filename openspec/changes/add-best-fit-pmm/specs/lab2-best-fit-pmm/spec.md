# Spec Delta

## Purpose

Lab2 内核使用 Best-Fit 策略管理连续物理页：分配时选择所有足够大的空闲块中最小的一个，释放时按地址归还并与相邻空闲块合并。

## ADDED Requirements

### Requirement: Use the best-fit manager

内核初始化物理内存管理时 MUST 选用 `best_fit_pmm_manager`，并在启动输出中打印 `memory management: best_fit_pmm_manager`。该管理器的自检 MUST 使用实验框架提供的 `best_fit_check`。

#### Scenario: Kernel boots with best-fit
- **WHEN** 内核启动并初始化物理内存管理
- **THEN** 输出 `memory management: best_fit_pmm_manager`，随后 `check_alloc_page() succeeded!`

### Requirement: Register initial free memory

对调用方保证全部处于保留状态的 n 页（n>0），初始化 MUST 将其登记为一个大小为 n 的空闲块：每页的标志、块大小和引用计数清零，首页记录块大小 n 并标为空闲块首页，空闲块按地址有序保存，空闲页总数增加 n。

#### Scenario: Initial memory map
- **WHEN** 对一段 n 页的保留内存执行初始化
- **THEN** 空闲页总数增加 n，并存在一个从该段首页开始、大小为 n 的空闲块

### Requirement: Allocate the smallest sufficient block

申请 n 页（n>0）时，分配器 MUST 在所有大小不小于 n 的空闲块中选择最小的一个；存在多个同样最小的块时可任选其一。块大于 n 时 MUST 从块前部切出 n 页返回，剩余部分仍作为空闲块保留在原有地址顺序中；块恰为 n 时整块返回。成功分配后空闲页总数减少 n，返回页不再标为空闲块首页。没有足够大的块或 n 大于空闲页总数时 MUST 返回空，且空闲块和空闲页总数不变。

#### Scenario: Best-fit differs from first-fit
- **WHEN** 空闲块为 {p0+1 起 2 页, p0+4 起 1 页}，依次申请 1 页和 2 页
- **THEN** 第一次返回 p0+4，第二次返回 p0+1

#### Scenario: Split a larger block
- **WHEN** 最小的足够块大小大于申请页数
- **THEN** 返回该块前部，剩余页仍为空闲块

#### Scenario: No block is large enough
- **WHEN** 没有大小不小于 n 的空闲块
- **THEN** 返回空，空闲状态不变

### Requirement: Free and coalesce pages

释放从 base 起的 n 页（n>0，均为已分配且非保留状态）时，分配器 MUST 清除这些页的标志和引用计数，将其作为大小为 n 的空闲块按地址有序归还，空闲页总数增加 n；与地址紧邻的前驱空闲块和后继空闲块 MUST 分别合并，合并后只有合并块的首页保留空闲块首页标志和正确大小。

#### Scenario: Restore a contiguous block
- **WHEN** 测试将 p0 起 5 页分批分配后执行 `free_pages(p0, 5)`
- **THEN** 空闲块恢复为一个从 p0 开始的完整 5 页块

### Requirement: Preserve the framework tests and interfaces

实现 MUST NOT 修改、移动或删除 `basic_check()` 与 `best_fit_check()`，MUST NOT 改变 `pmm.h` 公开接口，并 MUST 保留 `//lab2 function2-2` 标记注释。

#### Scenario: Framework check runs unchanged
- **WHEN** 内核或主机端测试运行框架提供的 `best_fit_check()`
- **THEN** 测试代码与框架原文一致且全部断言通过
