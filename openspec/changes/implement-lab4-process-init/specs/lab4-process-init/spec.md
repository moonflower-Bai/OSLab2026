## ADDED Requirements

### Requirement: 初始化进程管理

系统 SHALL 初始化进程链表及 PID 哈希表，创建使用启动栈和内核页表的 PID 0 空闲进程，并创建其可运行的 PID 1 内核线程。

#### Scenario: 初始化成功

- **WHEN** 进程管理初始化完成
- **THEN** current 指向 runnable 的 idleproc，initproc 可通过 PID 查找，且其父进程为 idleproc。

### Requirement: 创建内核线程

系统 SHALL 为新线程初始化 PCB、内核栈及首次运行现场，在成功登记后返回唯一 PID 并使其可运行；当前阶段地址空间复制是空操作。

#### Scenario: 创建成功

- **WHEN** 有可用进程名额和足够内存
- **THEN** 新线程登记在进程链表和 PID 哈希表中，拥有父进程与首次运行现场，返回其 PID。

#### Scenario: 资源不足

- **WHEN** 进程数达到上限或分配 PCB、栈失败
- **THEN** 返回对应负错误码，不登记不完整进程，释放已分配资源。
