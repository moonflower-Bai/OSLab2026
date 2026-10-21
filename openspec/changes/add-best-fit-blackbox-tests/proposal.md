# Proposal

## Why

`add-best-fit-pmm` 的验收只覆盖框架自带的 `basic_check` 与 `best_fit_check`，它们从不触发切分、真实合并、并列最小块和多数失败路径。需要一套与实现隔离的黑盒测试，在不修改项目代码的前提下检查 Best-Fit 分配器的不变量和边界行为。本 change 为已完成的测试工作补记规划记录：测试编写时用户限定只能写入 `test/`，当时没有创建 change。

## What Changes

- 新增 `test/lab2/` 黑盒测试套件，按 D1–D6 六个方向组织：D1 不变量巡检（B1–B5）及巡检器自检、D2 最小块选择边界、D3 合并路径全组合、D4 标志与引用计数残留、D5 随机模型验证、D6 失败路径无副作用；用户给出的 AT-P2-01 至 AT-P2-14 全部落地为用例。
- 新增 `test/lab2/inject_p2.py` 与 `test/lab2/run_p2.sh`：把 `lab2/` 复制到临时目录，只在副本中注入测试、构建并用 QEMU 启动，收集结果到 `test/lab2/results/`。
- 新增 `test/lab2/README.md` 说明用法、隔离方式与覆盖范围。

非目标：不修改 `lab2/` 下任何项目代码（包括测试失败时）；不读取分配器实现代码；不与 First-Fit 锁步比较；不实现或合并练习 1 的 `page_init()`；不修改 `doc/` 下的文档。

## Capabilities

### New Capabilities

- `lab2-best-fit-blackbox-tests`：在与项目代码隔离的副本中，以黑盒方式验证 Lab2 Best-Fit 分配器的不变量、选择、合并、标志和失败路径行为。

### Modified Capabilities

无。

## Impact

- 新增文件仅位于 `test/lab2/`。
- 依赖 RISC-V 交叉工具链与 QEMU；依赖 `doc/lab2/p2_blackbox_api.md` 列出的接口。
- 当前分支缺少练习 1 的 `page_init()`，直接运行会编译失败；可通过 `P2_PAGE_INIT_FROM=<commit>` 在副本中借用。
