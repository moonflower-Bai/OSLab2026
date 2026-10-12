# lab1 开发说明

本目录受根目录 `AGENTS.md` 约束；本文件只补充 lab1 的构建与运行要求。

## 工具链

- 构建需要 `riscv64-unknown-elf-gcc`、`riscv64-unknown-elf-ld`、`riscv64-unknown-elf-objcopy` 和 `riscv64-unknown-elf-objdump`。
- 手工运行需要 `qemu-system-riscv64`。
- 缺少工具时应明确报告环境条件，不得把未执行的验证写成通过。

## 验证

- 从仓库根目录运行 `make -C lab1`。命令必须自行终止；成功时退出码为 0，并生成 `lab1/bin/kernel`。
- `make -C lab1 qemu` 是交互式运行，预期控制台出现 `(THU.CST) os is loading ...`。退出 QEMU 使用 `Ctrl-A` 后按 `X`；自动化调用必须设置超时。
- 只有在完整评分环境可用时才运行 `make -C lab1 grade`，并如实报告结果。
