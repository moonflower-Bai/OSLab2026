# Tasks

## 1. 开发状态

- [x] 1.1 按用户后续要求保留 `Dev/Lab4/main`，通过 `git branch --show-current` 确认；不创建分支或提交。
- [x] 1.2 设置当前章节为 `lab4`，保留冻结列表，更新根 README 进度；核对 JSON 与实际目录、文档链接一致。

## 2. 环境与入口

- [x] 2.1 检查 GCC、binutils、QEMU、GDB、Make 和 Python 依赖；运行 `make -C lab4/lab4_new -j2` 并记录构建结果，仅修复必要的环境兼容问题。
- [x] 2.2 限时运行 `make qemu` 或其等价 QEMU 命令，使用独立临时目录执行现有 `make grade GRADE_TMP=<临时目录>`，记录启动与评分结果，区分环境故障和练习缺失。
- [x] 2.3 编写 `lab4/README.md`，包含源码入口、工具依赖、构建、运行、退出、评分及已知基线状态，逐项核对实际命令。
- [x] 2.4 将运行、调试与评分的 QEMU 加载参数统一为 `-kernel`，使用临时最小二进制验证实际 `make qemu` 入口，通过 `make -n debug` 和 `sh -n tools/grade.sh` 检查其他入口，确保评分断言与分数未变。

## 3. 最终验证

- [x] 3.1 执行 `openspec validate initialize-lab4-development --strict`、`python3 -B scripts/sync_ai_workflows.py --check`、`python3 -B scripts/check_governance.py --target Lab4` 和 `git diff --check`；结果及既有阻塞写入 `verification.md`，确认未改变实验练习与评分规则。

外部前提：本机已有可执行的 RISC-V GCC/binutils、QEMU、Make、Python 和 OpenSpec；评分若需要 GDB 则需相应 RISC-V 调试器。缺失依赖或既有基线故障如实记录，不以练习实现替代环境初始化。
