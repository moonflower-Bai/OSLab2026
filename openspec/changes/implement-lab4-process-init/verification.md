# 验证记录

- `sh lab4/lab4_new/tests/process-init/run.sh`：通过；使用 RISC-V Linux 编译器和 qemu-riscv64，其他内核依赖使用替身。
- 将框架 `tools/test_lab4.c` 中的 `test_lab4_process_create` 原样提取到临时隔离测试中执行：通过；这不是系统级启动测试。
- 在临时副本运行 `make grade`：测试注入成功，编译在缺少 Lab 3 中断和异常处理函数处失败；环境没有 `qemu-system-riscv64`，三个评分项均未运行，不能把 0/30 当作功能测试失败。
- 检查远端 `Dev/Lab4/baijingren/vmem` 的 42168c9：任务一实现存在但尚未合入 `Dev/Lab4/main`。在该提交的临时副本运行 `bash lab4/test/run_vmem_tests.sh`，九个宿主单元用例和四组模型测试通过；系统 QEMU 层缺少可执行文件而未运行，整套脚本返回非零。
- 该任务一实现的二级页表分配失败时会保留一个空的中间页表；其测试允许保留并在后续回收，是否需要立即回滚应与手册的资源清理要求一起确认。
