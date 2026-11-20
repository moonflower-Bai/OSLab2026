# Tasks

## 1. 章节与实现

- [x] 1.1 设置 lab4 为当前章节，检查 `lab-status.json` 和分支治理。
- [x] 1.2 实现进程初始化和线程创建；使用 RISC-V 编译器编译 proc.c，并以 `sh lab4/lab4_new/tests/process-init/run.sh` 验证创建及失败路径（需要 RISC-V Linux 静态编译器和 qemu-riscv64）。

## 2. 集成验证

- [x] 2.1 运行 `openspec validate implement-lab4-process-init --strict` 和 `python3 -B scripts/check_governance.py`；后者因集成分支已有的 `report/prompt.md` 路径不合规而失败。
- [x] 2.2 尝试 Lab 4 构建；现有的 Lab 3 中断处理未回填且缺失物理内存头文件，完整启动仍需其他模块和 QEMU system。
