# User Prompts

change: implement-lab3-trap-entry
reviewed_for_relevance: true
reviewed_for_sensitive_data: true

## 2026-10-27 14:22:23 +0800

## 功能模块：Trap 上下文恢复与 Trap 入口

### 实现位置
kern/trap/trapentry.S 中的 RESTORE_ALL 和 __alltraps。

### 已有条件
SAVE_ALL 已在当前内核栈上建立 struct trapframe。
执行完 SAVE_ALL 后，sp 指向 Trap frame 起始位置。
Trap frame 包含 36 个 REGBYTES 大小的槽位：
0～31 对应通用寄存器，32～35 对应 Trap 相关状态。
trap(struct trapframe *tf) 是已有的 C 语言处理接口。

### 实现目标
1. RESTORE_ALL 应从当前 Trap frame 恢复 Trap 前的处理器上下文。
2. __alltraps 应把当前 Trap frame 作为参数传递给 trap()。
3. trap() 返回后，应进入 __trapret，恢复上下文并通过 sret 返回。

### 约束
1. 不修改 SAVE_ALL。
2. 不修改 struct trapframe 的布局。
3. 不修改 trap() 的函数签名。
4. RESTORE_ALL 的读取偏移必须与 SAVE_ALL 的保存偏移一致。
5. x2/sp 保存的是 Trap 前的原始栈指针，必须按该含义恢复。
6. 恢复 sp 前，必须完成所有依赖当前 Trap frame 地址的读取。
7. 必须正确恢复 sstatus 和 sepc，保证 sret 能正确返回。
8. __alltraps 必须按 RISC-V 调用约定，通过 a0 传递 Trap frame 地址。

### 验收标准
1. trapentry.S 能够独立编译。
2. Trap 前后通用寄存器内容保持一致。
3. trap() 能收到正确的 Trap frame 地址。
4. trap() 返回后能够恢复原执行流，不重复触发同一断点异常。
5. 与练习 2 集成后，build、QEMU 和 grade 验证通过。

## 2026-10-27 14:22:23 +0800

你现在就直接根据这个提示词开始任务不就可以了吗

## 2026-10-27 14:34:07 +0800

允许兼容修正 (Recommended)

## 2026-10-27

[省略：含学号的实验标记补充与提交授权。]
