# lab2 开发说明

本目录受根 `AGENTS.md` 约束。实验代码直接位于本目录，报告位于 `report/`；沿用 `Dev/Lab2/main` 的目录布局。

## 工具和环境

- 编译需要 `riscv64-unknown-elf-gcc`、`ld`、`objcopy`、`objdump` 和 Make。
- 运行需要 `qemu-system-riscv64` 和能将内核与有效 DTB 交接给 S-mode 的 OpenSBI。
- 评分还需要 Python 3、POSIX shell、`bc`、`awk`、`sed` 和 `tar`。
- 本机可在仓库根目录运行 `source ../Lab1前置准备/activate.sh`，加载已准备的 QEMU、GDB 和 OpenSpec。该环境是本机依赖，不随实验源码提交。
- 本机 QEMU 包装层为 loader 启动选择 FW_JUMP。Lab2 必须额外验证 DTB 的有效性，不能仅凭固件横幅认定启动成功。

## 验证

从仓库根目录执行：

```bash
make -C lab2
timeout 15s make -C lab2 qemu
make -C lab2 clean
timeout 15s make -C lab2 test
timeout 60s make -C lab2 grade
```

- `make` 必须退出为 0 并产生 `lab2/bin/kernel` 和 `lab2/bin/ucore.img`。
- QEMU 内核会空转；外部超时 124 只表示限时结束。成功依据是 DTB 解析成功、`check_alloc_page() succeeded!` 及无 panic。
- `make test` 前必须清理，因为 Makefile 不跟踪编译宏的变化；最终预期 `grading: 6 / 6 points`。
- `make grade` 在临时副本注入评分测试。预期两项 PASS、`Total Score: 20/20` 和成功退出；保留输出后再清理本次启动的 QEMU，不能终止其他会话进程。
- 本机如需开环回调试端口，使用工具许可的网络权限；调试后清理本次进程。

## 实现与证据

- `basic_check()`、`default_check()`、`best_fit_check()` 和原始评分工具不得修改。
- 实现、日志、报告按阶段一起整理。日志不足以替代要求的真实终端截图。
- 编译产物、临时注入副本和工具安装不进入提交。
- 用户提示词证据仅保存在当前 change 的 `prompt.md`；课程所需 `report/prompt.md` 的最终导出需先解决根规则与门禁兼容问题。
