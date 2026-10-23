# hews 的 Lab2 阶段提交与协作交接

## 本次个人分支

- 分支：`Dev/Lab2/hews/page-init`；原始源码基线为 `d52989b`，推送前变基到最新集成分支 `e83e10e`。新增远端提交只调整元数据，实验源码一致。
- 负责内容：Sv39 启动身份映射、DTB 驱动的 `page_init()`、物理页管理数组与保留区、已有 First Fit 下的初始化验证。
- 必要公共基础：`best_fit_pmm.c` 中仅增加 `free_area` 和两个宏，解决原框架检查引用未声明变量导致的编译失败。
- Best Fit 算法、管理器启用和独立位图回归由其他组员负责，本分支没有引入其完整实现。

## 提交结构

1. `bb3dd37`：开发基线、章节元数据、工具说明和规划。
2. `be87b20`：启动映射、页初始化、必要公共声明及阶段验证。
3. `a59aed1`：两阶段 PR 均使用 Rebase and merge。
4. `docs(lab2)`：三人分工、个人阶段报告、review 和拆分后的验证交接。

完整参考实现仍保留在本地 `Dev/Lab2/hews/setup`，不会作为本次推送对象。组员参考 ZIP 位于仓库外，由用户自行发送。

变基前后的实验代码逐文件一致；原阶段提交 `ce803ae`、`896dde0` 的历史仍保留在本地参考分支。提交哈希变化不代表阶段记录丢失。

## 阶段验证

已有 [初始化构建记录](logs/page-init-build.log)、[启动记录](logs/page-init-qemu.log) 和 [GDB 定位](logs/boot-debug-gdb.log)。2026-10-21 拆分后重新验证：

| 检查 | 结果 | 证据 |
| --- | --- | --- |
| 清理后构建 | 退出 0，生成 RISC-V 内核 | [构建日志](logs/hews-build.log) |
| 默认 128 MiB 启动 | DTB 成功，32768 条记录、31929 个空闲页，First Fit 自检通过 | [启动日志](logs/hews-qemu-128m.log) |
| 64 MiB 启动 | DTB 成功，16384 条记录、15705 个空闲页，First Fit 自检通过 | [启动日志](logs/hews-qemu-64m.log) |

两次 QEMU 在成功输出后继续空转，均由外部 `timeout 12s` 终止，退出码为 124；通过依据是成功标记和无 panic。

本阶段运行中的管理器为 `default_pmm_manager`。`check_alloc_page() succeeded!` 表示已有 First Fit 下的自检通过，不能视为 Best Fit 或最终 20/20 评分通过。

## 其他组员如何接入

详见 [三人分工](division.md)。hews 的 PR 目标为 `Dev/Lab2/main`；建议初始化 PR 合入后，组员从更新后的集成分支开始。需要提前协作时，可以从本个人分支创建各自的 `Dev/Lab2/<成员>/<任务>`。

baijingren 负责选择与拆分，第三位成员负责登记、释放、合并和测试；共同修改 `best_fit_pmm.c` 时按函数范围提交。算法接通后，再将 `pmm_manager` 切换到 `best_fit_pmm_manager` 并运行原始内部检查、评分和独立回归。

个人分支 → `Dev/Lab2/main` → `Lab2` 两阶段均选择 **Rebase and merge**。本次用户授权包含个人提交与推送，不包含远端合并或创建发布分支。

## 课程最终交付仍待完成

- 核对完整指导书中的书面问题与 Challenge。
- 组员完成自己的模块、实际贡献说明、AI 工具信息和对应报告。
- 集成 Best Fit 后完成最终测试、截图及报告核对。
- 最终 `code/`、`report/` 布局与 `report/prompt.md` 导出仍需处理现有规则兼容问题。
