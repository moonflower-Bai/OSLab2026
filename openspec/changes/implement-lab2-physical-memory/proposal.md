# Proposal

## Why

用户指定的 `Dev/Lab2/main` 已在 `d52989b` 导入与原始目录一致的 Lab2 骨架，其中物理内存初始化和 Best Fit 分配器尚未实现；章节元数据仍是 Lab1。需要先明确导入、实现、验证和报告的推进顺序，使代码正确性与课程交付要求都有可检查的依据。

## What Changes

- 在 `Dev/Lab2/hews/page-init` 上交付 hews 的初始化部分，复用基线的 `lab2/` 骨架与章节元数据；原始目录与基线中的实验内容保持原样。
- 实现 `page_init()`：读取 DTB 内存信息、建立页描述符数组、保留已占用页并登记可分配页。
- 实现并启用 Best Fit 管理器，正确处理连续页分配、拆分、回收、相邻块合并和空闲页统计。
- 分别验证编译、QEMU 启动、框架自检、评分脚本和分配器边界场景，保存真实日志及截图。
- 按实验报告模板整理实现说明、提示词迭代、测试证据和原理分析，准备可审阅的 `code/`、`report/` 提交布局预览。
- 在实施前核对指导书练习正文；如出现额外必做题或 Challenge，先区分必做与选做，再更新本提案的范围与任务。

### Scope

本 change 描述小组的整体目标。2026-10-21 用户要求只推送 hews 对应分工，因此当前提交分支为 `Dev/Lab2/hews/page-init`，只包含启动映射、物理页初始化与相关公共基础和文档；Best Fit 算法、独立回归与最终集成由组员负责，相关任务保持待完成。完整实现另存本地 `Dev/Lab2/hews/setup` 并导出参考 ZIP，不作为本个人分支的已交付功能。

### Non-goals

- 保留启动框架结构，修复验证所必需的 Sv39 取指映射；不改框架测试或评分逻辑，不将 First Fit、伙伴系统或动态页表管理作为额外实现目标。
- 不改变基线对 Lab1 的处理，不自行恢复或删除其他实验内容，也不自行冻结 Lab1。
- 不在本 change 中扩大或放宽仓库治理规则；提交目录和 `report/prompt.md` 与现有门禁的冲突在交付前单独审阅解决。
- 按授权创建阶段 commit；不自动创建发布分支、推送、归档或执行远端合并。

## Capabilities

### New Capabilities

- `lab2-physical-memory`: DTB 驱动的物理页初始化与连续页 Best Fit 分配、释放行为。
- `lab2-verification`: Lab2 开发布局、可复现验证、结果证据与报告交付预览。

### Modified Capabilities

无。根说明沿用已有 `root-readme` 的内容组织规则；提示词证据按当前 `AGENTS.md` 和 `prompts/README.md` 保存，不在本次规划中迁移或修订历史归档规范。

## Impact

- 预期代码位置：`lab2/kern/mm/pmm.c`、`lab2/kern/mm/best_fit_pmm.c`。
- 配套位置：`lab2/AGENTS.md`、`lab2/report/`、`lab-status.json` 和根 `README.md` 的实际进度说明。
- 工具依赖：RISC-V 交叉工具链、QEMU、OpenSBI、GDB、Make、Python、`bc`、OpenSpec。已有 `../Lab1前置准备/bin/` 可提供 QEMU、GDB 和 OpenSpec；需核验 Lab2 的 DTB 交接，不能仅凭 Lab1 启动记录断言可用。
- 验收依据：骨架内的 `basic_check()`、`best_fit_check()`、`tools/test_lab2.c` 和 `tools/grade.sh`，以及本地课程交付说明和报告模板。
- 尚未取得指导书练习正文，因此不声称已覆盖其中所有书面问题或 Challenge。
