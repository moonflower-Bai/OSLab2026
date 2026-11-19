# Tasks

## 1. 记录与章节状态

- [x] 1.1 创建 `implement-lab4-process-scheduling` 的 proposal、design、spec、tasks 和 prompt 证据。
- [x] 1.2 将 `lab-status.json` 的 `current` 设为 `lab4`。

## 2. 实现模块三

- [x] 2.1 在 `kern/process/switch.S` 实现 `switch_to`。
- [x] 2.2 在 `kern/process/proc.c` 实现 `proc_run`。
- [x] 2.3 在 `kern/process/proc.c` 实现 `cpu_idle`。
- [x] 2.4 在 `kern/schedule/sched.c` 实现 `schedule`。

## 3. 验证

- [x] 3.1 每阶段运行对象级编译：`switch.o`、`proc.o`、`sched.o`，并用 `objdump`/`nm` 检查符号和偏移。
- [x] 3.2 在仓库外临时组合模块一/二和最小 Lab3 返回链，构建内核并运行官方 `check Schedule`。
- [x] 3.3 运行 `openspec validate implement-lab4-process-scheduling --strict` 和仓库治理检查；环境缺失时如实报告。

## 4. 提交边界

- [ ] 4.1 每阶段只 stage 该阶段文件，由用户执行 commit；不 push，不提交临时依赖、构建产物或队友代码。
