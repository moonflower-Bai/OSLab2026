# Proposal

## Why

用户已提供完整练习正文并明确要求实现 SLUB 扩展。已有页分配接口不能直接高效提供小于一页的对象，需要在其上增加对象分配层，并用独立测试验证任意请求长度、复用、回收和失败行为。

## What Changes

- 从最新 `origin/Dev/Lab2/main` 的 `7074bb6` 创建 `Dev/Lab2/hews/slub`，保留原工作区的 review 和答辩文档。
- 新增简化 SLUB：固定大小缓存、slab 内空闲对象链、部分空闲 slab 管理、空 slab 归还和大对象连续页回退。
- 修复扩展回归暴露的 First Fit 尾插遍历缺少停止条件的问题，保留原框架自检函数。
- 在物理页初始化后接入对象分配器及独立自检，增加主机回归和非法调用检查。
- 报告保存两份 AI 实现／测试提示词、覆盖范围、实际结果和局限，按阶段准备可审阅的提交分组。

### Scope

用户本次指令已授权实现与验证，不等待额外 apply。底层复用当前管理器的 `alloc_pages()/free_pages()`，不将 Best Fit 或 Buddy 的小组任务一起实现。

### Non-goals

- 不修改现有 First Fit、Best Fit 自检和原评分工具，不改 `struct Page` 布局。
- 不实现 Linux 的全部 SLUB 接口、per-CPU、NUMA、并发／中断安全和 GFP 策略。
- 保留原 review 内容并补充扩展；按用户后续明确授权提交并推送个人分支，不归档或合并远端 PR。

## Capabilities

### New Capabilities

- `lab2-slub`: 基于页分配器的任意长度对象申请与归还，以及可复现的扩展验证。

### Modified Capabilities

无。

## Impact

新增 `lab2/kern/mm/slub.h`、`slub.c`、`slub_test.c` 和主机验证工具；在 `kern/init/init.c` 中接入，补充 Make 主机测试目标。`default_pmm.c` 的初始化登记与释放尾插各补一个 `break`。报告与日志位于 `lab2/report/`。最终 PR 目标为 `Dev/Lab2/main`，按仓库规范选择 Rebase and merge。
