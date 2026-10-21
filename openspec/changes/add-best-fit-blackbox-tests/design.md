# Design

## Context

用户要求测试与项目代码隔离、只读接口文档不读实现、只写 `test/`。参考样例 AT-P2-NN 通过直接改写 `free_list`/`nr_free` 隔离现场，但 `doc/lab2/p2_blackbox_api.md` 说明这两个名字是 `best_fit_pmm.c` 内的 static 符号，外部无法链接。仓库已有 `lab2/tools/inject.py` 示范如何把测试注入构建。

## Goals / Non-Goals

**Goals:**
- 在不改项目代码的前提下覆盖框架检查未触及的路径。

**Non-Goals:**
- 不让测试替代 `make grade` 的课程验收。

## Decisions

1. **用“全部取走再划试验区”代替改写 `free_list`/`nr_free`**。先用 `alloc_pages` 取走全部空闲页，在其中划出 64 页试验区并两端各留 1 页 guard，用 `free_pages` 构造状态。这是 API 文档规定的唯一可行办法；各样例的断言内容保持不变。
2. **注入到临时副本**。`inject_p2.py` 仿照 `inject.py`，把测试复制到副本的 `kern/test`，修改副本 Makefile，并在 `pmm_init` 末尾调用；项目目录只读。
3. **QEMU 改用 `-kernel` 启动**。Makefile 的 `-device loader` 在本机 QEMU 11 / OpenSBI 1.8 上不进入内核（`Next Address = 0`），脚本在副本运行时改用 `-kernel`。
4. **`P2_PAGE_INIT_FROM` 借用 `page_init()`**。当前分支缺练习 1 的实现，脚本可在副本中用指定提交的 `pmm.c`/`entry.S` 替换后再打回本分支对 `pmm.c` 的改动；被测的 `best_fit_pmm.c` 保持原样。
5. **随机模型用位图**。D5 在测试侧维护页位图，返回块只需属于“最小可容纳块”集合，避免对并列选择做不必要的约束。
6. **`P2_MANAGER=default` 自检**。换成基线 First-Fit 应当出现失败，用于确认检查会报错。

## Risks / Trade-offs

- [基线 `default_pmm.c` 是未完成的教学骨架，First-Fit 自检失败多来自链表/合并检查而非选块] → 只能说明检查会报错，不能证明能区分两种策略；结论如实记录。
- [借用的 `page_init()` 来自队友分支] → 仅用于副本运行；合并 page-init 后应在不借用的情况下重跑。
- [QEMU 启动参数与 Makefile 不同] → 影响平时 `make qemu`，单独处理，不在本 change 修改项目文件。
