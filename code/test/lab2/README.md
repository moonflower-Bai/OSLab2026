# Lab2 P2 Best-Fit 黑盒测试

只用 `doc/lab2/p2_blackbox_api.md` 列出的接口，不引用 `free_list`、`nr_free` 或分配器的 static 函数。运行时先把 `lab2/` 复制到 `$TMPDIR`，在副本里注入测试，项目源码不改。

```sh
sh test/lab2/run_p2.sh                         # 全部用例一次启动；失败时自动逐例重跑
sh test/lab2/run_p2.sh --per-case              # 每个用例单独启动
P2_PAGE_INIT_FROM=be87b20 sh test/lab2/run_p2.sh   # 本分支缺 page_init 时借用 P1 提交
P2_MANAGER=default sh test/lab2/run_p2.sh      # 自检：换成 First-Fit，应当失败
```

结果写入 `test/lab2/results/`（`latest.log` 是最近一次）。

## 文件

| 文件 | 作用 |
|---|---|
| `p2_blackbox/test_p2_blackbox.c` | 测试本体，`P2_CASE=k` 只跑第 k 个，`0` 全跑 |
| `inject_p2.py` | 仿照 `lab2/tools/inject.py`：复制到副本的 `kern/test`，改副本 Makefile，在 `pmm_init` 末尾调用 |
| `run_p2.sh` | 复制、注入、构建、QEMU 启动、收集结果 |

## 隔离方式

按 API 文档的做法：用 `alloc_pages` 取走全部空闲页，在取到的段里划出 64 页试验区 `[arena, arena+64)`，两端各留 1 页已分配作 guard。每个用例开始和结束时试验区全部处于已分配状态，且 `nr_free_pages()==0`。全部结束后按原样归还，并断言 `nr_free_pages()==baseline`。断言失败会打印用例名、行号和试验区页状态，然后 panic，不尝试修复损坏状态。

## 覆盖

| 方向 | 用例 |
|---|---|
| D1 不变量巡检 | 每次 alloc/free 后执行 `scan()`：B1 环上节点与 `PG_property` 首页一一对应且按地址升序、只有一个哨兵、`next->prev` 一致；B2 首页 `0<property<=剩余页数`、块内页无 `PG_property`/Reserved、`ref==0`；B3 `Σproperty==nr_free_pages()`；B4 块不重叠；B5 相邻块必须已合并。另有 `D1 checker self-test`：故意改坏 5 种页状态，确认巡检器都能报错，随后原样恢复 |
| D2 选择边界 | AT-P2-01/02/03/04/13，最小块在头部、尾部、中间，三方并列，并列且需要切分，唯一恰等块 |
| D3 合并组合 | AT-P2-05/06/07/08，合并链（前合、后合、双侧合并，最后整块分配） |
| D4 标志残留 | AT-P2-09/10/14，切分与乱序合并后逐页检查，`free_pages` 清零 `ref`；每次 alloc/free 都检查涉及的每一页 |
| D5 随机模型 | 3 个种子，各 1500 次操作。测试侧位图模拟 Best-Fit：返回块必须是“最小可容纳块”之一（并列时任选其一都合法），空闲块布局必须等于模型的极大连续段。不与 First-Fit 对拍 |
| D6 失败无副作用 | 空集合分配，`n>nr_free`，`n` 等于总量但分散，`n` 小于总量但没有单块能容纳，AT-P2-11/12。每次失败后逐页比较 flags/ref/首页 property |

样例 AT-P2-NN 原文直接改 `free_list`/`nr_free` 做隔离，这里改用 API 文档规定的“全部取走”方式实现，断言内容保持一致。
