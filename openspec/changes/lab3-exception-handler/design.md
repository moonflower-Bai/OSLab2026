# Design

## Context

见 `proposal.md` 的 Why。`trap_dispatch` 已在 cause 的最高位为 0 时调用 `exception_handler`，该函数目前为空。`trapentry.S` 的 `SAVE_ALL` 已把 32 个通用寄存器和 status、`epc`、badvaddr、cause 放到栈上，`RESTORE_ALL` 仍是空宏。`trap_in_kernel` 以 `sstatus.SPP` 判断是否来自内核。`print_trapframe` 会打印 `cause    0x%08x`。`uintptr_t` 是 `unsigned long long`。通过条件见 `specs/lab3-exception-handling/spec.md`，并与 `lab3/test/exception.sh` 的宿主机层对齐。

## Goals / Non-Goals

**Goals:**

- 只在断点和非法指令上读取 `epc` 处的第一条 16 位半字，并据此推进 `epc`。
- 未知异常复用 `print_trapframe`，panic 与否只调用 `trap_in_kernel`。
- `__alltraps` 在 `SAVE_ALL` 之后把 `sp` 作为参数调用 `trap()`。

**Non-Goals:**

- 不重写规格中的通过条件。
- 不补 `RESTORE_ALL`，不改 `SAVE_ALL`，不实现 `interrupt_handler`。

## Decisions

### 只识别 cause 2 和 cause 3

`CAUSE_ILLEGAL_INSTRUCTION` 为 2，`CAUSE_BREAKPOINT` 为 3。其余非负 cause，包括 0、1、4 到 16、20、31、63，都走未知异常。中断不会进入 `exception_handler`，因为 `trap_dispatch` 已经按 cause 的符号位分流。

备选是在处理函数里再次判断最高位。这会重复已有分派，不采用。

### 指令长度只看第一条半字的低两位

低两位为 `0b11` 时 `epc` 加 4，否则加 2。读取发生在打印原始 `epc` 之后、写回 `epc` 之前，并且只出现在上述两个分支。未知 cause 不解引用 `epc`，因此 `epc` 为 0 或指向不可读页时不会因此失败。

备选是固定加 4，或按 48/64 位指令的 `inst[4:2]` 继续加长。规格要求的前进量只由第一条半字的低两位决定，不采用这些备选。

### 地址用 `%016llx` 打印

宿主机上的指令地址可以高于 4GiB。`%08x` 只留下低 32 位，数值比较会失败。`uintptr_t` 是 `unsigned long long`，`0x%016llx` 在内核格式化和宿主机 `printf` 中都取出 64 位，打印全部比特。两种异常使用各自的专属字符串，不调用 `print_trapframe`，避免混入另一类字符串。

备选是 `%p`。它在内核里按指针取值，和宿主机桩的格式解释不一定相同，不采用。

### 未知异常先打印，再按 `trap_in_kernel` 决定 panic

默认分支调用 `print_trapframe`。`trap_in_kernel(tf)` 为真时接着 `panic`，为假时直接返回。不修改 `epc` 或其他 trapframe 字段。`panic` 使用现有宏，从而进入 `__panic`。

备选是凡未知异常都 panic，或都不 panic。规格要求两种 `SPP` 的结果相反，不采用。

### 汇编只补调用

`SAVE_ALL` 结束时 `sp` 就是 trapframe。`__alltraps` 用 `mv a0, sp` 和 `jal trap` 进入已有的 `trap()`，返回后 `sp` 不变，再落到 `__trapret`。不读取 `scause`，不做中断与异常分流。`SAVE_ALL` 与 `RESTORE_ALL` 的宏体不改。`__alltraps` 里的练习占位注释删掉，不留下 `TODO`。

备选是在汇编里按 `scause` 分别调用两个处理函数。用户要求分流留在 `trap_dispatch`，不采用。

### 不处理用户提供的内容

本变更只处理陷阱帧里的 cause、status 和 `epc`。没有新的用户输入、凭证或持久化记录。打印的地址沿用内核控制台输出，不新增采集或外传。

## Risks / Trade-offs

- [空的 `RESTORE_ALL` 不能恢复寄存器] → 本次按范围不补该宏。默认验收只运行宿主机检查并链接 QEMU 层，不启动镜像。设置 `EXCEPTION_RUN_QEMU=1` 仍会在恢复处失败，这不作为本次通过条件。
- [`%016llx` 带前导零] → 比较的是完整数值，前导零不改变数值。
- [只实现异常处理，时钟中断仍为空] → `interrupt_handler` 保持空函数。内核启用中断后的计时行为不在本次验收内。

## Migration Plan

修改上述两个文件即可。回滚时恢复这两个文件在本次实现之前的内容。不迁移数据，不改测试文件。

## Open Questions

无。`RESTORE_ALL` 留空是范围决定，不是待定问题。
