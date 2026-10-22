# Tasks

2026-10-21 分工调整：当前分支只交付 hews 的启动与页初始化。Best Fit 与最终集成任务由组员后续完成，保持未勾选；完整参考代码另外打包。

## 1. Confirm scope and execution prerequisites

- [x] 1.1 核对指导书 `lab2_2_1_exercise.html`、`lab2_5_labs_requirement.html` 与本地提交说明；在设计记录中列出已确认必做项、选做项和无法取得的正文，未知内容不计作已覆盖，新增实质要求另行确认。
- [x] 1.2 加载本机已有工具环境，验证 GCC、Binutils、QEMU、GDB、OpenSpec、Make、Python 和 `bc` 可用；版本命令须成功，记录 QEMU 包装层的固件行为及尚待验证的 DTB 交接。

本机环境加载命令（在仓库根目录执行）：

```bash
source ../Lab1前置准备/activate.sh
riscv64-unknown-elf-gcc --version
riscv64-unknown-elf-ld --version
riscv64-unknown-elf-objcopy --version
riscv64-unknown-elf-objdump --version
qemu-system-riscv64 --version
riscv64-unknown-elf-gdb --version
openspec --version
make --version
python3 --version
bc --version
```

## 2. Prepare the isolated development tree

- [x] 2.1 复用用户指定 `origin/Dev/Lab2/main` 的 `lab2/` 骨架，保留源目录；用逐文件内容比较确认基线与原始文件一致，并记录骨架尚未实现导致的预期编译问题。
- [x] 2.2 将 `lab-status.json.current` 更新为 `lab2`，保留 `frozen` 原值并同步根 README 的实际进度；核对 `git diff origin/Dev/Lab2/main -- lab1` 为空、当前开发分支属于 Lab2。
- [x] 2.3 添加 `lab2/AGENTS.md` 的工具、构建、自检和运行限时说明，创建 `lab2/report/report.md` 的模板结构；逐项核对命令路径，成员和分工未知时明确留待补充。

## 3. Implement and verify physical memory initialization

- [x] 3.1 补齐 Best Fit 编译单元所需的共享空闲区声明与宏，实现 `page_init()` 并修复必要的 Sv39 启动取指映射，暂时保留 First Fit；`make -C lab2` 须退出码为零并产生 RISC-V 内核 ELF。
- [x] 3.2 核对 DTB 内存范围、偏移、`npage` 与 `npage - nbase` 的含义、页描述符对齐和保留区；限时运行 QEMU 须出现合理内存信息及 First Fit 的 `check_alloc_page() succeeded!`，日志无 panic。
- [x] 3.3 在报告中补写初始化模块的布局、计算依据、实际提示词迭代及该阶段验证结果；检查描述与代码、日志相符，不提前写 Best Fit 或最终评分通过。

本阶段验证命令：

```bash
make -C lab2
riscv64-unknown-elf-readelf -h lab2/bin/kernel
timeout 15s make -C lab2 qemu
```

QEMU 命令可能因正常空转返回 124；必须检查成功标记、panic 和停止原因，不能单独依据退出码判断启动成功。启动异常时先核对固件、`boot_dtb` 和内核映射；临时调试进程须在本次检查后清理。

## 4. Implement and verify Best Fit

- [ ] 4.1 实现 Best Fit 初始化、内存块登记、最小足够块选择、拆分、回收合并和统计，并定义及启用 `best_fit_pmm_manager`；普通编译及启动须通过原始 `basic_check()`、`best_fit_check()`。
- [ ] 4.2 清理后执行 `make test`，检查内部进度达到 `6 / 6`；核对精确匹配、较大块拆分、最小块选择、失败返回、前后合并和计数恢复的覆盖，必要的额外场景在独立临时副本验证，原框架保持原文。
- [ ] 4.3 在报告中补写 Best Fit 的选择依据、链表不变量、与 First Fit 的比较及实际测试结果；核对解释能对应实现和测试证据。

本阶段验证命令：

```bash
make -C lab2 clean
make -C lab2
timeout 15s make -C lab2 qemu
make -C lab2 clean
timeout 15s make -C lab2 test
```

## 5. Run integration validation and capture evidence

- [ ] 5.1 执行原始 `make grade` 并保存完整输出；两项测试均 PASS、总分 `20/20`、命令成功且日志无 panic 才标记评分通过，检查并清理本次遗留进程。
- [x] 5.2 使用 64 MiB 内存执行一次启动验证；2026-10-21 在个人分支以已有 First Fit 复验，DTB 范围、16384 条描述符和 15705 个空闲页正确，自检通过；日志见 `hews-qemu-64m.log`。Best Fit 的集成验证仍待组员完成。
- [ ] 5.3 对照原始骨架检查框架函数及 `tools/grade.sh`、`tools/inject.py`、`tools/test_lab2.c` 未被修改；运行本 change 的严格校验与仓库治理检查，记录失败原因并修复范围内问题。
- [ ] 5.4 从实际终端运行取得启动、自检和评分截图，保存到 `lab2/report/images/` 并补齐报告的集成验证部分；逐一检查图片相对链接及截图和日志的对应关系。

集成验证命令（每个新终端先加载工具环境）：

```bash
timeout 60s make -C lab2 grade
make -C lab2 clean
make -C lab2
timeout 15s make -C lab2 qemu QEMU='qemu-system-riscv64 -m 64M'
openspec validate implement-lab2-physical-memory --strict
python3 -B scripts/check_governance.py --branch Dev/Lab2/hews/page-init --target Dev/Lab2/main --self-test
git diff --check
```

## 6. Review report and prepare the submission handoff

- [ ] 6.1 核对报告模板的目的、环境、整体逻辑、各模块、实际迭代、测试和总结均有对应内容；检查已确认练习的覆盖情况，列出缺失的指导书内容、成员信息及待补项，明确区分代码完成与课程交付完成。
- [ ] 6.2 完成 `lab2/ → code/`、`lab2/report/ → report/` 的提交预览说明，并记录 `report/prompt.md` 与当前规则的冲突及必要的后续审阅范围；不生成被禁止的受版本管理提示词副本，不把该冲突标记为已解决。
- [ ] 6.3 汇总改动、原始测试一致性、真实验证结果和交付待办供审阅；检查 Lab1 与仓库外骨架无改动。后续按 `dev → stable → release` 准备分支流转，commit、推送、归档、发布分支创建和合并遵从届时已有明确授权，本任务以交接清单完成为准。
