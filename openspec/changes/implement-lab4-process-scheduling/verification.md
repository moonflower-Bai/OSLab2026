# 验证记录

## 对象级编译

- `make -C lab4/lab4_new obj/kern/process/switch.o`：通过。
- `make -C lab4/lab4_new obj/kern/process/proc.o`：通过。
- `make -C lab4/lab4_new obj/kern/schedule/sched.o`：通过。
- `objdump` 确认 `switch_to` 保存/恢复 14 个上下文槽位；`nm` 确认 `switch_to`、`proc_run`、`cpu_idle`、`schedule` 符号。

## 仓库外集成验证

临时目录：`/tmp/lab4-sched-e2e`（不进入仓库）。

组合方式：

- 以 `origin/Dev/Lab4/baijingren/vmem` 为模块一/Lab2 基线。
- 用 `origin/Dev/Lab4/hews/process-init` 的 `proc.c` 提供模块二。
- 应用本次模块三的 `proc.c`、`sched.c`、`switch.S` diff。
- 仅在临时树补齐 Lab3 的 `RESTORE_ALL`、`__alltraps`、`interrupt_handler`、`exception_handler`。
- 仅在临时树修正官方测试中 `void (*)(void *)` 与 `int (*)(void *)` 的 GCC 16 编译错误。
- 临时跳过会在该组合基线中卡住的 `check_boot_pgdir()` 自检；该自检不属于模块三。

构建：

```sh
make -C /tmp/lab4-sched-e2e/lab4 -j2
```

通过，生成 `bin/kernel` 和 `bin/ucore.img`。

官方评分：

```sh
cd /tmp/lab4-sched-e2e/lab4
sh tools/grade.sh
```

结果：

```text
-check PMM:                                OK
-check Process Create:                     OK
-check Schedule:                           OK
Total Score: 30/30
```

调度相关输出：

```text
[Test Schedule] PASS
this initproc, pid = 1, name = "init"
To U: "Hello world!!".
To U: "en.., Bye, Bye. :)"
kernel panic at kern/process/proc.c:209:
    process exit!!.
```

最后的 `process exit` panic 来自模块二保留的 `do_exit` 桩，发生在三行输出和调度测试通过之后；不属于模块三。

## OpenSpec

- `openspec validate implement-lab4-process-scheduling --strict`：通过。
- `openspec validate --all --strict`：14 passed, 0 failed。
- `openspec validate --archived --strict`：3 passed, 0 failed。
- `scripts/check_governance.py` 仍因基线已有的 `report/prompt.md` 路径失败；该失败不是本 change 引入。

## 未提交内容

仓库外基线、队友模块一/二代码、Lab3 shim、测试兼容性修改、`check_boot_pgdir` 跳过和所有构建产物均未进入仓库提交。
