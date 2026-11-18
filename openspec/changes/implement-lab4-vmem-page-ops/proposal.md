# Proposal

## Why

Lab4 功能一的页操作只有声明，尚不能建立或撤销虚拟地址到物理页的映射。需要补齐实现，使叶项、物理页引用和缓存翻译在成功与失败路径上保持一致，并运行已有的三层验收。

## What Changes

- 实现 `page_insert()` 的首次映射、同页更新、替换和内存不足处理。
- 实现 `page_remove()` 的撤销和末次引用回收，确保改变页表后、回收旧页前完成相应的本地地址翻译失效。
- 原样运行独立的单元、QEMU 集成和模型仿真测试；另外报告已知的完整内核构建阻塞。

## Capabilities

### New Capabilities

- `lab4-vmem-page-ops`: Lab4 的 Sv39 叶项管理、数据页引用计数、失败处理与本地地址翻译同步。

### Modified Capabilities

无。

## Impact

实现仅改动 `lab4/kern/mm/pmm.c` 中两项页操作及必要的 TLB 辅助逻辑。保留 `pmm.h` 接口、`get_pte()` 框架、物理页分配策略、自检、验收脚本及评分规则。不扩展到中间表回收、多 hart 同步或其他模块，不读取 `lab4/tools/`，不调用主构建或评分入口。
