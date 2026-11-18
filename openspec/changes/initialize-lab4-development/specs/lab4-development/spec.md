# Spec Delta

## Purpose

使已有 Lab4 源码具有一致的当前章节状态和可复现的本地开发入口。开发者能够据此找到源码、构建并尝试运行现有骨架，同时区分环境是否可用与实验练习是否完成，避免初始化误报实验验收通过。

## ADDED Requirements

### Requirement: Lab4 is the active chapter

初始化后的仓库 MUST 将当前章节声明为 `lab4`，保留既有冻结列表；根说明 MUST 准确呈现本分支中已有章节，并链接 Lab4 的使用说明。

#### Scenario: Developer checks the active lab
- **WHEN** 开发者读取章节状态和根说明
- **THEN** 当前章节均为 Lab4，现有源码入口指向 `lab4/lab4_new/`，冻结列表不变

### Requirement: Reproducible environment instructions

Lab4 使用说明 MUST 给出工具依赖、源码路径、构建、运行、退出和评分命令，MUST 如实区分环境验证结果与未完成练习导致的失败。

#### Scenario: Developer follows the local setup
- **WHEN** 开发者从项目根目录按 Lab4 说明操作
- **THEN** 能进入真实源码目录并使用既有构建、QEMU 和评分入口；已知阻塞及验证结果有明确记录

### Requirement: Initialization preserves exercise behavior

初始化 MUST 保留现有实验练习与评分规则，不以实现练习或降低验收标准消除基线失败。

#### Scenario: Baseline contains unfinished exercises
- **WHEN** 构建、启动或评分因原有练习缺失而失败
- **THEN** 报告具体阶段和原因，保留练习原状，并明确环境验证的实际边界

### Requirement: QEMU receives the kernel entry

本地运行、调试与评分入口 MUST 将内核二进制作为固件启动负载传给 QEMU，使默认固件获得有效内核入口，且 MUST 保留现有评分标准。

#### Scenario: Default firmware boots a known valid payload
- **WHEN** 使用本机 QEMU 和默认固件启动已验证的最小二进制
- **THEN** 固件下一入口为内核加载地址，程序可输出串口标记；运行、调试与评分使用相同加载方式
