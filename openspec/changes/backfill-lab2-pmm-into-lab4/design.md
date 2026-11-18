# Design

## Context

Lab4 的启动汇编用一个 Sv39 根级叶表项把从 `DRAM_BASE` 开始的 1 GiB 映射到内核虚拟地址。Lab4 已有设备树解析、中断保护、启动页表、自检与堆初始化。Lab3 的历史 change 回填的是 Lab3 目录，此次需要将同一 Lab2 成果接入 Lab4 的框架。

## Goals / Non-Goals

目标是恢复 Lab2 的物理内存初始化和 Best-Fit 分配行为。接口边界止于物理页分配与回收，页表映射操作和其他练习分别实施。

## Decisions

- 以本地远程跟踪引用 `origin/Dev/Lab2/main`（`d3eb836`）为源。它与 `origin/Lab2`（`c6fbd66`）的 `pmm.c`、`best_fit_pmm.c` 对应 blob 完全相同，本地 `Dev/Lab2/main` 仍是未填充骨架。
- 提取 Lab2 的 `page_init()`。用 Lab4 的 `PDX1SHIFT` 表达同一个 1 GiB 映射上限。Lab4 现有 `pmm_init()`、中断保护和页表测试的调用顺序继续作为集成入口。
- 把 Lab2 的 Best-Fit 分配器主体及操作表迁入 Lab4 的 `default_pmm.c`。操作表使用 Lab4 已声明的 `default_pmm_manager` 符号，名称仍标识 Best-Fit。Lab4 自带的 `basic_check()`、`best_fit_check()` 留在原文件。该文件使用自己的 `default_pmm.h`。
- 临时副本用原 Lab4 入口时停在进入 C 代码之前，使用历史 Lab2 根页表的恒等映射后通过分配器自检。迁入同一个 Sv39 根级叶表项来维持分页启用到高地址跳转之间的取指。跳转后清除低地址表项并刷新 TLB，使进入 C 代码时仍使用 Lab4 原有的高地址映射。
- 完整内核受未完成模块影响。先直接编译两个实际目标文件，再在临时副本中将启动流程截到物理内存自检，运行原有断言。随后尝试未裁剪的 Lab4 构建并记录第一个实际阻塞。

## Risks / Trade-offs

- [历史 Lab2 文件使用专属管理器声明和 Sv39 常量] → 对照 Lab4 启动映射和现有声明进行适配。
- [整文件覆盖会带入不同的启动检查顺序] → 仅提取源实现片段，核对自检函数及现有架构代码的 diff。
- [用户请求中含个人分支标识] → Prompt 原文使用省略记录。
