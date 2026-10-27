# Design

## Context

见 `proposal.md` 的 Why。当前 `lab3/kern/trap/trap.c` 在 cause 非负时调用 `exception_handler`，该函数尚未定义；`trapentry.S` 的 `RESTORE_ALL` 和进入 `trap` 的调用仍是练习空位。`trap()` 已存在，`print_trapframe` 会打印 `cause    0x%08x`。`panic` 是展开为 `__panic` 的宏。宿主机是 macOS，不能执行 RISC-V 的 `csrr` / `csrw`。用户限制测试只新增 `lab3/test/` 下的文件，并且不读取 `lab3/tools/`。

## Goals / Non-Goals

**Goals:**

- 宿主机直接链接仓库里的 `trap.c`，用桩替换 `cprintf`、`__panic` 和 `interrupt_handler`。
- 失败信息只携带实测值。
- QEMU 层以可编译的源码写明保存/恢复和委托检查，本次入口不调用它。

**Non-Goals:**

- 不在设计里重写规格中的通过条件。见 `specs/lab3-exception-tests/spec.md`。
- 不把对抗变体或参考实现写入仓库。

## Decisions

### 宿主机编译 `trap.c`，不使用 QEMU 用户态

宿主机层明确不依赖 QEMU。用宿主机编译器编译 `trap.c`：定义 `__riscv_xlen=64` 以通过头文件中的字长分支，并用强制包含的宏把 `read_csr` / `write_csr` 变成空操作。不定义 `__riscv`，因此 `riscv.h` 里的真实 CSR 汇编不会启用。

备选是 `qemu-riscv64` 运行交叉编译的测试。该方案依赖 QEMU，与宿主机层的约束冲突，不采用。

### 经 `trap()` 进入异常处理

`trap_dispatch` 是 `trap.c` 内部函数。测试调用已导出的 `trap()`，这样无论 `exception_handler` 以后写成外部函数还是 `static`，都走仓库里的那份实现。中断桩只为满足链接；非负 cause 不应调用它。

当前 `exception_handler` 写在调用点之后。宿主机编译器会把先出现的调用当成隐式 `int` 声明，再和后面的 `void` 定义冲突。测试用强制包含的原型与现有非 `static` 定义对齐，并在原型上标 `weak`，使定义本身变成弱符号。测试里的 `interrupt_handler` 强符号因此替换内核里的空定义；`EXCEPTION_EXTRA_SRC` 里的强符号同样替换 `exception_handler`。不使用 `llvm-objcopy`。

宿主机不能编译 `libs/atomic.h` 的 RISC-V `amo` 汇编。测试用 `-include` 提供等价的 C 位操作并占住该头的保护宏，不把同名 `atomic.h` 放进更靠前的 `-I` 目录。交叉编译 QEMU 镜像时仍使用仓库里的 `libs/atomic.h`。

### 桩与输出捕获分开放到不包含内核 `defs.h` 的编译单元

内核 `defs.h` 自定义了 `bool`、`size_t` 和定宽整数，不能和宿主机的 `stdio.h`、`signal.h` 放在同一个编译单元。运行时单元负责：

- `cprintf` 用宿主机格式化把结果追加到缓冲区；
- `__panic` 记录后 `siglongjmp`，使调用方能继续下一项；
- 不可读页上的 `SIGSEGV` 同样跳出，用来观察未知 cause 是否读取了 `epc`。

`cause == 20` 且帧被清零时 `epc` 为 0，读取会表现为故障。其余未知 cause 把 `epc` 指到 `PROT_NONE` 页。断点和非法指令使用测试映射的本地编码，地址不为 0。

### 失败报告不携带期待值

检查函数只接收条件、名称和“当前值”字符串。条件为假时打印名称和当前值。不把期待值格式化进字符串，也不使用恒真断言。

### 地址按完整数值比较

宿主机无法把指令页固定映射到 4GiB 以下。输出里的地址必须与传入的 `epc` 数值相等，因此格式需要打印全部比特。

### QEMU 层由脚本链接

`exception.sh` 用 `riscv64-unknown-elf-gcc` 编译 `qemu_exception.c`、`qemu_exception.S`，再用 `ld --wrap=trap --wrap=kern_init` 把它们和仅用于链接检查的桩连成可执行文件。`__wrap_kern_init` 调用测试，`__wrap_trap` 在进入 `trap()` 时记录 `tf`、`sp`、`sscratch`、`scause`、`sstatus`，以及 trapframe 槽位 0 的 `x0`。完整内核镜像另用 `lab3/tools/kernel.ld` 链接，启动命令写在 `exception.sh` 开头。`EXCEPTION_RUN_QEMU=1` 且给出 `EXCEPTION_QEMU_IMG` 时才启动 QEMU。当前 binutils 不认识 `trapentry.S` 里的 `sbadaddr`，因此默认不把尚未能汇编的内核推进这条链接。

### 对抗变体留在临时目录

默认脚本只编译仓库里的 `trap.c`。验证时用环境变量 `EXCEPTION_EXTRA_SRC` 额外链入一份临时 `exception_handler`。该变量不是默认路径。变体和参考实现都不进入仓库。子代理只生成变体，不读取 `lab3/test/`。

## Risks / Trade-offs

- [隐式函数声明在新编译器上变成错误] → 为 `trap.c` 提供与定义一致的原型，不关闭该诊断。
- [低地址映射被系统拒绝，`%08x` 与 64 位 `epc` 不相等] → 比较完整地址；打印全部比特的实现可以通过。
- [宿主机无法编译真实 `atomic.h`] → 仅宿主机翻译单元用 C 位操作占住保护宏；QEMU 镜像仍编译真实头文件。
- [QEMU 运行依赖尚未完成的 `RESTORE_ALL`] → 默认只编译和链接；需要启动时设置 `EXCEPTION_RUN_QEMU=1`。
- [捕获的 `cprintf` 文本可能含地址] → 只作为本次进程的测试输出，不写入仓库。对抗变体放在临时目录。`prompt.md` 只保存已经人工确认、不含敏感值的用户原文。

## Migration Plan

新增测试文件即可使用。回滚时删除 `lab3/test/exception.sh`、`lab3/test/exception/` 和本 change 目录。不迁移内核状态。

## Open Questions

无。当前未知项不影响规格、方案或任务划分。
