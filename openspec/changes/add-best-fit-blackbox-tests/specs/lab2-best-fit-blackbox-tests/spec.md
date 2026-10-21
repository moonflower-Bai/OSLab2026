# Spec Delta

## Purpose

在与项目代码隔离的临时副本中，以黑盒方式验证 Lab2 Best-Fit 物理内存分配器的不变量、最小块选择、合并、标志残留与失败路径行为，并留下可复查的运行日志。

## ADDED Requirements

### Requirement: Isolate tests from project code

测试套件 MUST 只在 `test/` 下保存自身文件，运行时 MUST 把 `lab2/` 复制到临时目录并只在副本中注入、构建和启动；运行和失败 MUST NOT 修改 `lab2/` 下任何文件。测试代码 MUST 只使用 `doc/lab2/p2_blackbox_api.md` 列出的接口，MUST NOT 引用 `free_list`、`nr_free` 或分配器的 static 函数。

#### Scenario: A test fails
- **WHEN** 某个用例断言失败
- **THEN** 运行脚本报告失败，`git status` 显示 `lab2/` 无任何修改

### Requirement: Isolate each case inside an arena

每个用例 MUST 在从真实分配器取得的试验区内构造状态，试验区两端 MUST 各有一页保持已分配的 guard。用例开始和结束时试验区 MUST 全部处于已分配状态；全部用例结束后 MUST 归还所有页，并断言空闲页数等于运行前的基线。断言失败时 MUST 打印用例名、位置和试验区页状态后停止，MUST NOT 尝试修复损坏状态。

#### Scenario: Suite completes
- **WHEN** 全部用例通过
- **THEN** 最终空闲页数等于运行前基线

### Requirement: Check invariants after every operation

每次分配和释放后，测试 MUST 检查以下不变量：B1 空闲链表节点与空闲块首页一一对应、按地址严格升序且前后指针一致；B2 块首页 `0<property<=剩余页数`，块内其余页无空闲块首页标志、无保留标志且引用计数为 0；B3 各块大小之和等于空闲页数；B4 块不重叠；B5 地址相邻的空闲块已合并。巡检器 MUST 带自检：对每种故意破坏的状态都能报错。

#### Scenario: Checker self-test
- **WHEN** 测试故意制造违反 B2、B3、B4、B5 的页状态
- **THEN** 巡检器对每一种都报告违规

### Requirement: Cover selection, coalescing, flag and failure paths

测试 MUST 覆盖：D2 最小块位于链表头、尾、中间，多个同尺寸最小块并列（任一选择都合法），唯一块恰好等于请求，切分方向与余块位置；D3 只与前驱合并、只与后继合并、前后同时合并、无相邻块不合并，以及合并后再分配的完整性；D4 分配和释放后逐页检查标志、块大小与引用计数；D6 空链表分配、`n` 大于空闲页数、`n` 等于总量但分散、`n` 小于总量但没有单块可容纳时返回空且状态逐页不变。用户给出的 AT-P2-01 至 AT-P2-14 MUST 各有对应用例且断言语义一致。

#### Scenario: First-fit regression
- **WHEN** 空闲块为 {3@p0, 2@p0+4}，依次申请 2 页、2 页
- **THEN** 第一次返回 p0+4，第二次返回 p0，余块为 {1@p0+2}

#### Scenario: Scattered free pages
- **WHEN** 空闲块为 {2@p0, 3@p0+4}，申请 5 页
- **THEN** 返回空，链表、块大小与空闲页数与申请前一致

### Requirement: Validate against a best-fit model

D5 MUST 使用多个固定随机种子执行随机分配和释放序列，以测试侧模型模拟 Best-Fit：每次返回块 MUST 是当前最小可容纳块之一，空闲块布局 MUST 等于模型的极大连续空闲段，空闲页守恒且块不重叠。测试 MUST NOT 与 First-Fit 锁步比较成功或失败。

#### Scenario: Random sequence
- **WHEN** 以固定种子执行随机操作序列
- **THEN** 每一步的返回块和空闲布局都与 Best-Fit 模型一致

### Requirement: Report results

运行脚本 MUST 输出每个用例的通过或失败以及汇总行，并把日志写入 `test/lab2/results/`（`latest.log` 为最近一次）。构建失败 MUST 作为失败报告，并在日志中保留编译错误。

#### Scenario: Branch cannot build
- **WHEN** 被测分支缺少编译所需的函数
- **THEN** 运行脚本报告构建失败并保存错误日志，不报告任何用例通过
