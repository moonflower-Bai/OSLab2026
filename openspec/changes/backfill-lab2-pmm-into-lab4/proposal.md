# Proposal

## Why

Lab4 的两个 Lab2 预留位置尚未填充。模块一的页表操作依赖能通过自检的物理页初始化和分配器，因此先迁移已完成的 Lab2 代码。

## What Changes

- 迁移前从当前 Lab4 稳定分支创建并切换到用户指定的开发分支，保留工作区里的初始化改动。
- 从已完成的 Lab2 分支迁入基于设备树的物理内存初始化和 Best-Fit 页分配器，接入 Lab4 现有 PMM 框架。
- 迁入 Lab2 启用分页时所需的恒等映射，并在跳转到内核高地址后撤销，使启动流程能到达物理内存初始化。
- 使用 Lab4 自带的分配器自检验证迁移。模块一的页映射实现、Lab3 中断回填、后续进程练习与评分规则属于其他工作。

## Capabilities

### New Capabilities

- `lab4-pmm-backfill`：在 Lab4 提供源自 Lab2 的物理页初始化、分配与回收行为。

### Modified Capabilities

无。

## Impact

实现范围为 `lab4/lab4_new/kern/mm/pmm.c`、`lab4/lab4_new/kern/mm/default_pmm.c` 和 `lab4/lab4_new/kern/init/entry.S`。迁移复用 Lab4 已有设备树接口、页描述符和 PMM 管理器声明。
