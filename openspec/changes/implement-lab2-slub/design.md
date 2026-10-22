# Design

## Context

基点 `7074bb6` 含已集成的启动映射及 `page_init()`，目前选用 First Fit。SLUB 通过公共页接口分层，不替换 `pmm_manager`。当前运行环境是单 hart、无并发调用的 Lab2 内核。

## Decisions

1. 提供 `slub_init()`、`kmalloc(size_t)`、`kfree(void *)`。初始化在 `pmm_init()` 返回后进行，不依赖堆分配。
2. 16、32、64、128、256、512、1024、2048 字节八种缓存，统一 16 字节对齐。slab 使用一页，头部放自身管理信息，空闲对象内部保存下一空闲对象指针。
3. 每个缓存只维护有剩余对象的 partial 链表；填满时移出，释放一个对象时重新加入。所有持有的 slab／大对象另有登记链表，用于不解引用外来地址的归属查找。
4. 每个 slab 用占用位图检查槽位状态。最后一个对象释放时，立即脱链并归还底层页。登记链表查找为线性复杂度，保留教学可读性，不宣称 Linux SLUB 快路径性能。
5. 大于 2048 字节的请求，用头部加请求长度向上取整为连续页；记录申请页数，释放时向同一公共接口归还。底层若按规格扩大量分配，须遵守其自身申请／释放配对契约，本扩展不读取其内部块字段。
6. 零长度和容量不足返回 NULL；加法溢出先检查；`kfree(NULL)` 无动作。非法归还通过 panic 拒绝，调用者仍须维护所有权；地址再次使用后无法识别旧指针的历史身份。
7. 独立自检只依赖公开接口，覆盖尺寸边界、数据互不覆盖、full/partial 复用、跨 slab、归还总量、真实页耗尽和恢复。主机额外验证非法调用，并在 ASan/UBSan 下执行固定种子的混合操作。

## References

参考 Linux 官方 [内存分配指南](https://docs.kernel.org/core-api/memory-allocation.html) 的页／对象分层和 [v6.12 SLUB 源码说明](https://raw.githubusercontent.com/torvalds/linux/v6.12/mm/slub.c) 中的 freelist、partial slab 思想。本实现从本实验接口重新编写，不复制 Linux 源码；省略其 per-CPU、锁及 NUMA 机制。

## Validation and commit structure

- 阶段一：First Fit 尾插修复；针对性回归与第三阶段测试一起交付。
- 阶段二：SLUB 实现、启动接入、行为规格和实现提示词。
- 阶段三：独立内核／主机测试、Make 入口和测试提示词。
- 阶段四：真实构建、128/64 MiB 启动及主机检测结果、报告与 review 材料。

2026-10-22 用户明确授权测试通过后 commit 并 push。按上述四组拆分，验证每组暂存快照可编译，再推送 `Dev/Lab2/hews/slub`。既有的 review/defense 修改放入文档分组，原始框架测试与评分工具逐字节保持不变。不创建发布分支、不归档或合并 PR；最终 PR 选择 Rebase and merge。

## Debugging evidence

首轮主机回归在第 32 个随机步骤检测到大对象与存活小对象重叠；QEMU 在耗尽回收阶段未完成。GDB 检查到 PMM 计数已恢复而空闲链表尾节点状态错误。原因是 `default_init_memmap()` 和 `default_free_pages()` 在 `list_add` 到尾部后继续遍历，下一轮再次处理新节点并执行自身插入。各增加一个 `break` 后，同一回归、7 类预期 panic 和两种 QEMU 配置均通过。主机适配器补充了分两段递增登记、递增归还及完整连续区恢复的定向检查。

新代码没有改变原 `basic_check()`／`default_check()`，也未改变 Best Fit 自检或评分工具。主机构建最初额外启用 `-Wextra` 导致原框架符号比较告警；主机入口最终采用与内核一致的 `-Wall -Werror`，没有为了消除告警改写原测试。
