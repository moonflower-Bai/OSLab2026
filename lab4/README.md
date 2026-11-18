# Lab4：进程管理

当前章节由根目录 `lab-status.json` 指定为 `lab4`。源码和 Makefile 位于 `lab4/lab4_new/`；以下命令从仓库根目录开始。

## 本机环境

初始化时已确认以下工具可执行，无需重新安装：

| 工具 | 本机版本 |
| --- | --- |
| RISC-V GCC | 16.2.0 |
| RISC-V binutils | 2.47.20260726 |
| RISC-V GDB | 18.1 |
| QEMU | 11.1.2 |
| GNU Make | 3.81 |
| Python | 3.9.6 |

默认交叉编译前缀为 `riscv64-unknown-elf-`，需要对应的 `gcc`、`ld`、`objcopy`、`objdump`；调试和现有评分脚本还使用对应的 `gdb`。编译、链接、二进制转换及 QEMU 串口输出已通过独立临时最小程序验证。

## 构建和运行

```sh
cd lab4/lab4_new
make -j2
make qemu
```

输出文件为 `bin/kernel` 和 `bin/ucore.img`。QEMU 使用 `virt` 机器、默认 OpenSBI 固件和 `-kernel bin/ucore.img` 加载二进制。退出时先按 `Ctrl+a`，松开后再按 `x`。

工具链前缀不同时，可使用现有变量覆盖，例如 `make GCCPREFIX=riscv64-elf-`；后续运行和评分应使用同一前缀。`CMakeLists.txt` 不是本实验的实际构建入口，请使用 Makefile。

## 调试和评分

在源码目录中运行：

```sh
make debug
```

另一个终端进入相同目录后运行 `make gdb`。调试入口监听本机 1234 端口并等待 GDB 连接。

评分会清理生成的构建产物、复制源码，并在临时副本中注入已有测试；可指定独立临时目录：

```sh
grade_dir=$(mktemp -d "${TMPDIR:-/tmp}/lab4-grade.XXXXXX")
make grade GRADE_TMP="$grade_dir"
```

成功运行后评分目标会删除该临时副本；失败时可保留副本用于定位。评分脚本还使用 `awk`、`bc`、`date`、`grep`、`sed` 等命令。

## 当前骨架状态

2026-11-17 初始化验证时，工具链与 QEMU 最小程序验证通过，完整实验尚不能构建或评分：

- `kern/trap/trap.c` 标有补充 Lab3 代码的位置，缺少 `interrupt_handler` 和 `exception_handler`，构建首先在此失败。
- 进程管理和上下文切换仍有练习占位；现有评分注入器因找不到 `proc_init` 而停止。
- 已将运行、调试和评分入口统一为 `-kernel`。本机 QEMU 11.1.2 配合原有 `-device loader` 启动最小程序时，默认固件没有获得有效内核入口；使用 `-kernel` 后串口输出成功。

这些结果不代表 Lab4 练习已完成。补齐实验代码后需重新执行构建、启动和评分。详细验证记录见 [初始化验证记录](../openspec/changes/initialize-lab4-development/verification.md)。
