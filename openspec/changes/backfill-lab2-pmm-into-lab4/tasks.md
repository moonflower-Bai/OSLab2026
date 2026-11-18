# Tasks

## 1. 分支和代码迁移

- [x] 1.1 从当前 HEAD 创建并切换到用户指定的 Lab4 开发分支，核对分支名、HEAD 和初始化 diff。
- [x] 1.2 提取 Lab2 的 `page_init()` 和 Best-Fit 分配器实现，按 Lab4 的宏和管理器声明接入；对照来源和迁移 diff，检查两个原有自检函数完整保留。
- [x] 1.3 迁入 Lab2 启动期恒等映射并在高地址跳转后撤销；在临时 QEMU 副本使用工作区的入口汇编，进入 C 时断言低地址根表项已清除。

## 2. 验证

- [x] 2.1 执行 `make -C lab4/lab4_new obj/kern/mm/pmm.o obj/kern/mm/default_pmm.o`，验证迁移模块编译。在临时副本通过 QEMU 运行现有分配器自检，验证设备树内存发现和 `check_alloc_page() succeeded!`。
- [x] 2.2 运行 `make -C lab4/lab4_new -j2`、`git diff --check` 和 `openspec validate backfill-lab2-pmm-into-lab4 --strict`，核对 Lab4 工作区范围及目标分支，记录实际结果和仍独立存在的整内核阻塞。

外部依赖为 RISC-V GCC 和 binutils、GNU Make、QEMU、Python 3 及 OpenSpec。

验证记录（2026-11-18）

- 两个 PMM 目标文件及 `obj/kern/init/entry.o` 编译通过。历史源代码对比确认仅适配 Lab4 的页表常量、管理器声明和启动入口，原有分配器自检完整保留。
- 原 Lab4 启动入口在 QEMU 11.1.2 上停在 C 初始化之前。使用工作区新入口的临时副本在 `kern_init()` 入口确认根表索引 2 的恒等映射已清除、索引 511 的高地址映射仍有效。
- 临时副本只运行到 `pmm_init()` 中的 `check_alloc_page()`，并用 `DEFS=-Ducore_test` 显示原有 Best-Fit 自检分数。128 MiB 设备树解析成功，自检输出 `grading: 6 / 6 points` 和 `check_alloc_page() succeeded!`，QEMU 正常退出。临时副本跳过后续页表、堆、Trap 与进程初始化。
- 完整 `make -C lab4/lab4_new -j2` 仍因 `trap.c` 中缺少 Lab3 的两个处理函数，以及 `kmalloc.c:92` 把整数传给需要 `void *` 的 `kva2page()` 而停止。
- OpenSpec 严格校验、AI 工作流检查、修改范围、分支目标、当前 change 的 Prompt 格式、行尾以及 `git diff --check` 均通过。
