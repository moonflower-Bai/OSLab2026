# Verification

验证日期：2026-11-17。本次保留 `Dev/Lab4/main`，未提交、推送或归档。当前章节为 `lab4`，冻结列表仍为空。

## Environment

GCC 16.2.0、binutils 2.47.20260726、GDB 18.1、QEMU 11.1.2、GNU Make 3.81、Python 3.9.6 和 OpenSpec 1.13.1 可执行。

临时验证目录为 `/private/tmp/lab4-init-04vvub0b/`，不纳入项目提交。目录包含最小汇编程序、交叉编译产物及本次命令日志。最小程序直接向 `virt` 串口写入 `LAB4_TOOLCHAIN_QEMU_OK`，不依赖实验练习。

## Build and boot

| 验证 | 结果 |
| --- | --- |
| `make -C lab4/lab4_new -j2` | 失败：原有 `trap.c:102`、`:107` 调用缺失的中断和异常处理函数 |
| 原有 `make qemu` | 在上述构建阶段停止，未启动实验内核 |
| `make grade GRADE_TMP=<本次独立临时目录>` | 失败：既有注入器无法定位缺失的 `proc_init`；尚未进入评分执行 |
| 临时程序 GCC 编译、链接及 objcopy 转换 | 通过 |
| 原始 `-device loader` 加载临时二进制 | 固件下一入口为零，未输出标记 |
| `-kernel` 加载同一临时二进制 | 通过：固件入口 `0x80200000`，输出标记 |

最小程序使用 `-march=rv64imac_zicsr_zifencei -mabi=lp64 -mcmodel=medany -nostdlib -nostartfiles -Wl,-Ttext=0x80200000 -Wl,-e,_start` 编译，随后转为原始二进制。QEMU 测试均限时运行并终止本次创建的进程。

修正后的实际 Makefile 入口使用 `make -C lab4/lab4_new -o /private/tmp/lab4-init-04vvub0b/smoke.bin qemu UCOREIMG=/private/tmp/lab4-init-04vvub0b/smoke.bin` 验证。`-o` 仅用于复用已编译的临时程序，以单独检查启动入口，不代表实验内核已能构建。

该实际启动入口通过，串口输出 `LAB4_TOOLCHAIN_QEMU_OK`；调试入口 dry-run 保留 `-kernel` 与 `-s -S`，评分脚本 `sh -n` 通过。评分脚本 diff 仅有一行加载参数变化，未修改断言和分数。

## Governance

| 检查 | 结果 |
| --- | --- |
| `openspec validate initialize-lab4-development --strict` | 通过 |
| `python3 -B scripts/sync_ai_workflows.py --check` | 通过，24 份提示词 |
| `git diff --check` | 通过 |
| 当前章节、修改范围、stable 分支目标、当前 change Prompt 格式及文档链接检查 | 通过 |
| `python3 -B scripts/check_governance.py --target Lab4` | 未通过，仅报告既有 `report/prompt.md` 路径不被允许 |

全仓库治理脚本内部执行的活动与归档 OpenSpec 严格检查未报告错误。本次不读取、迁移或修改上述历史 Prompt 文件。完整实验编译和评分仍受未完成练习阻塞，不宣称验收通过。

## Scope

实验功能及评分断言、分数保持原状。实验配置修改限于 Makefile 的 QEMU 运行、调试加载参数和现有评分脚本的同一加载参数。其余为章节状态、文档和当前 change 记录。
