# lab3 开发说明

本目录受根 `AGENTS.md` 约束。代码来自官方 `lab3.zip`（`lab3_new`），导入时未改动，练习位置以 `lab3` 与 `YOUR CODE` 注释标记。

## 工具和环境

- 编译需要 `riscv64-unknown-elf-gcc`、`ld`、`objcopy`、`objdump` 和 Make。
- 运行需要 `qemu-system-riscv64`；评分需要 Python 3 和 POSIX shell。

## 当前状态

练习（`kern/trap/trapentry.S`、`kern/trap/trap.c`）尚未实现，此时 `make -C lab3` 因缺少 `exception_handler` 而失败，属预期。

## 验证

练习完成后，从仓库根目录执行：

```bash
make -C lab3
timeout 15s make -C lab3 qemu
timeout 60s make -C lab3 grade
make -C lab3 clean
```
