# Lab1 交付检查与个人提交范围

检查时间：2026-10-08。Git 依据为本地 main 的 11cc128 和已有远端分支缓存；当前已创建个人规划分支 Dev/Lab1/hews/prepare-submission，尚未创建 commit 或推送。

## 要求依据

1. 仓库外的《实验基本信息.txt》：一个小组使用一个公开 Git 仓库，每个实验使用 labx 分支；Lab1 交付包含 code/ 和 report/，后者包含 report.md、prompt.md、images/。
2. 仓库外的《实验报告模板.md》：基本信息与分工、目的、环境及每人的 AI 工具/模型、整体分析、逐题解答与真实迭代过程、运行截图、OS 原理联系和 AI 协作收获。
3. 指导书 Lab1 练习页：http://8.135.34.58/lab2026/_book/lab1/lab1_2_1_exercise.html 。报告要求页：http://8.135.34.58/lab2026/_book/lab1/lab1_5_requirement.html 。本地 HTML 只保存概览正文和目录，这两页本次仍访问失败；练习二、Challenge 和特定截图要求未确认。
4. OSLab2026/AGENTS.md、README.md、lab1/AGENTS.md：开发使用 Dev/*，经 PR 合入 main；只改当前章节；OpenSpec 提案、实现、归档、提交分别授权。它们是小组工作流规则，不是老师题目。
5. 《Lab1内容规划与分工.md》仅是前置讨论稿，不代表最终成员分工或完整官方要求。

## 交付目录

```text
lab1 分支
code/                         最终源码、Makefile、链接脚本及验证工具
report/
  report.md                   按课程模板完成的 Markdown 报告
  prompt.md                   用户明确提供并经人工审阅的课程 Prompt
  images/                     报告引用的真实测试截图
```

这些是明确要求的目录；课程通知未明确说明是否禁止额外文件，最终分支不能仅凭本检查决定保留哪些治理元数据。

## 仓库实际缺项

| 项目 | 已有内容 | 缺项或待核对 |
| --- | --- | --- |
| 公开仓库 | 已配置 GitHub origin | 本次网络读取失败，公开状态未核实 |
| lab1 交付分支 | 本地 main 和队友开发分支缓存 | 本地未见 lab1；远端实时状态未核实 |
| 最终代码 | lab1/ 原始骨架；21 个源码/构建文件与 ZIP 对应文件一致 | 未逐项核对完整题目；尚无 code/ 导出 |
| 实验报告 | 队友开发分支的 doc/lab1/main.tex | 练习一有草稿；练习二和多节正文为空；main 无报告；缺 report.md |
| 课程 Prompt | 仓库已有 OpenSpec 证据规则 | 缺课程汇总；现有 CI 拒绝 report/prompt.md，不能直接新增后声称可提交 |
| 测试截图 | 仓库外文本日志 | 缺报告所引用的真实终端截图；LaTeX 校徽不是测试证据 |
| 复现工具 | 仓库外 activate.sh、包装脚本、双终端脚本 | Git 克隆者拿不到这套本机环境；缺仓库内可移植的验证入口 |
| 构建与启动证据 | 本会话在仓库外副本重编译及启动成功 | 需要形成个人可提交的命令、输出和说明 |
| GDB 证据 | 前置日志观察到 PC、两个入口和启动栈 | 本会话还未重新执行 GDB；不能描述为个人已完成所有练习 |
| 评分 | Makefile 有 grade 目标 | ZIP 和仓库均无 tools/grade.sh；评分未执行 |
| CI | 治理检查和 main ruleset JSON | 当前 CI 未验证内核构建/启动；远端保护规则是否启用未核实 |
| 个人贡献 | 已创建本人的规划开发分支 | 尚无本人的实验实现或验证工具 commit、push、PR |

## 建议的第一份个人提交

建议沿用前置分工建议，承担环境复现、构建/启动验证和 GDB 启动观察，交付如下真实贡献；最终分工仍由小组确认：

- lab1/scripts/check_env.sh：检查实际工具与架构支持，明确报告缺失条件。
- lab1/scripts/verify_boot.py：构建并有限时长启动 QEMU，以内核横幅判断启动结果。
- lab1/scripts/verify_debug.py：在回环地址上启动调试，验证复位 PC、kern_entry、bootstacktop 和 kern_init。
- lab1/README.md：别人从仓库复现的命令、环境要求和退出方式。
- lab1/report/hews-validation.md：个人环境、观察、固件适配问题和实际验证范围。
- 对应的小体积文本日志及真实截图；OpenSpec 规格与当前 change 的合法 Prompt 证据一起提交。

当前骨架已具备最小启动功能，个人贡献可以是可复现验证工具和证据。只有正式题目要求或实际错误需要时才修改内核；不能把原始骨架重新复制一份称为个人实现。

已有课程通知没有明确要求每位成员都必须修改内核函数。个人提交需要准确体现自己的贡献；是否另有个人代码考核规定，仍应核对正式课程要求。

## 答辩准备范围

以下依据已读源码、指导书目录和日志整理，不保证老师只问这些：

| 主题 | 需要能解释的内容 | 主要对应文件 |
| --- | --- | --- |
| 交叉编译 | 主机与目标架构；GCC、ld、objcopy 的职责；.o 与 ELF、raw binary 的区别 | Makefile、tools/function.mk |
| 链接布局 | kern_entry 为什么是入口；0x80200000、段布局、页对齐、edata/end | tools/kernel.ld |
| 启动交接 | QEMU 负责加载镜像；复位 0x1000、OpenSBI 基址 0x80000000、内核入口 0x80200000；M/S 模式 | Makefile、启动与 GDB 输出 |
| 启动栈 | la 伪指令、bootstacktop、8 KiB 栈、为何进入 C 前设置 sp | kern/init/entry.S、kern/mm/memlayout.h |
| 尾调用 | tail 的效果、ra 与不返回的初始化函数、反汇编可能不同于伪指令写法 | entry.S、kern_init 反汇编 |
| C 初始化 | memset(edata,0,end-edata)、输出横幅、while(1)；本次构建 edata=end，清零长度实际为零 | kern/init/init.c、ELF 符号 |
| 输出路径 | cprintf、格式处理、cons_putc、SBI、ecall | kern/libs/stdio.c、libs/printfmt.c、kern/driver/console.c、libs/sbi.c |
| 调试证据 | PC/sp、断点、单步；区分继续运行命中入口与完整单步固件 | GDB 脚本与日志 |
| 个人贡献 | 哪些是课程骨架、队友材料、自己的脚本；复现方式与验证限制 | Git diff、commit、报告 |

队友草稿将加载内核归于 OpenSBI，这处应在报告整合时改为：QEMU 加载镜像，OpenSBI 初始化后交出控制权。

## 提交顺序与边界

1. 先核对正式练习页并落实分工；已确认的验证工具可以独立实现。
2. 按本 change 的 tasks 实现个人脚本和材料，运行有限时长的构建、启动、调试验证。
3. 补齐组内 Markdown 报告、真实截图、AI 使用记录和课程 Prompt；无法确认的评分结果据实说明。
4. 审查 diff 和验证结果，明确授权后创建个人 commit、推送并发起 PR；本地 commit 和 GitHub 可见提交是两个不同步骤。
5. 完整材料导出为 code/report，检查最终目录后再按明确授权准备 lab1 交付分支。治理元数据的保留方式需在最终分支创建前确定。

大型 .tools/、.downloads/、node_modules/、构建生成的 obj/bin 和临时调试文件不属于推荐提交范围。课程指定的截图应作为报告证据提交。
