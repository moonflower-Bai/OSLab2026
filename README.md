# OSLab2026

2026 秋季操作系统实验，内核是 riscv64 ucore。指导书在 [lab2026](http://8.135.34.58/lab2026/_book/)。

每一章单独一份骨架，放在 `lab1/`、`lab2/` 这样的目录里。下一章不会在上一章的目录上继续改，旧章节提交后就不再动。

## 当前进度

`lab1/` 是最小可执行内核。框架代码已经给全，练习是阅读启动流程，并用 GDB 从复位跟到 `0x80200000`。

本地另存了两份环境说明：`0_environment_setup.md`、`3_startdash.md`。

## 构建 lab1

需要 `riscv64-unknown-elf-gcc` 和 `qemu-system-riscv64`。

```bash
cd lab1
make
make qemu
```

终端出现 `(THU.CST) os is loading ...` 后，按 `Ctrl-A` 再按 `X` 退出 QEMU。`make qemu` 不会自己结束。

调试时开两个终端：

```bash
make debug
make gdb
```

## CI

只检查正在做的那一章。换章时改 workflow 里的目录。
