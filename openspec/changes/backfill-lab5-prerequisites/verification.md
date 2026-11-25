# 验证结果

- 工具链为 RISC-V GCC 16.1.0 与 QEMU 10.2.2。迁移基线为 `origin/Dev/Lab4/main`（`d4bb252`），前置分支参考为 `origin/Dev/Lab2/main`（`d3eb836`）和 `origin/Dev/Lab3/main`（`547cb32`）。
- `make -C lab5 -j2` 编译了本次修改涉及的内核目标。完整构建停在 `kern/syscall/syscall.c:11`，系统调用练习中的 `sys_exit` 等函数尚未定义。
- `riscv64-unknown-elf-objdump -dr lab5/obj/kern/trap/trapentry.o` 确认了 288 字节陷阱帧、状态和返回地址偏移、`sscratch` 的双路径、最后恢复 `sp` 以及 `sret`。
- 从仓库根目录执行 `sh openspec/changes/backfill-lab5-prerequisites/verify-trap.sh`。脚本在临时副本中绕过系统调用练习，并启动 QEMU virt 与 OpenSBI。Best-Fit、页表、启动映射和 VMM 自检通过，定时中断超过 100 次。内核线程切换后回到原线程，S 态断点可恢复。通过 `forkrets` 进入 U 态后两次执行 `ecall`，检查全部整数寄存器、用户栈和返回地址。每次用户陷阱处理期间触发一次 S 态断点，断点返回后外层陷阱帧完整。QEMU 通过 SBI 正常关机。
- `openspec validate backfill-lab5-prerequisites --strict` 与 `git diff --check` 通过。当前改动的章节范围、分支、行尾与本 change 的 Prompt 格式检查通过。仓库级 `python3 -B scripts/check_governance.py --target Lab5` 仍报告历史 Lab4 VMM change 的 Prompt 时间格式，以及 `report/prompt.md` 的遗留路径。
