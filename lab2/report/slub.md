# Lab2 SLUB 扩展报告与问答

开发分支：`Dev/Lab2/hews/slub`，基于 `Dev/Lab2/main` 的 `7074bb6`。本次只完成 SLUB 扩展，通过公共物理页接口接入，当前底层仍为 First Fit。

## 第一份 prompt：实现模块

下面是根据本次任务设计的实现提示词。它是课程要求的可复用设计材料，不是另一段用户原文或另一次独立模型调用的记录；本次在同一 Codex 会话中按实现、测试两个阶段执行。

```text
请阅读 lab2/kern/mm/pmm.h、memlayout.h、pmm.c、default_pmm.c、
libs/list.h 和 kern/init/init.c，在当前 ucore 框架中从零实现简化 SLUB。

目标是两层内存分配：底层继续调用现有 alloc_pages/free_pages，
上层通过 kmalloc(size_t size)、kfree(void *ptr) 分配任意正长度对象。
新增 slub_init，在 pmm_init 返回后初始化并接入独立自检。

请采用 16～2048 字节的 2 次幂大小缓存，返回地址至少 16 字节对齐。
每个小对象 slab 占一页，页内包含元数据及空闲对象链表；缓存跟踪
partial slab，分满时移出、释放后重新进入，最后一个对象归还时释放整页。
用槽位占用状态与所有活跃 slab 登记信息校验归还，不修改 struct Page。
大对象使用带头部的连续页申请，记录实际请求给底层的页数以便配对释放。

明确 kmalloc(0) 返回 NULL；检查 size 加头部的溢出；无法取得页时
返回 NULL 且不破坏存活对象、不泄漏页；kfree(NULL) 无动作。
非法对象指针与可检测的重复释放通过 panic 拒绝，不读取任意外来地址。
仅支持当前无并发调用场景，说明元数据、内部碎片和查找复杂度。

请编写易于 review 的 C 代码和接口注释，保留现有 pmm_manager 的选择，
不得修改 basic_check/default_check/best_fit_check 和原评分工具。
不要引入 libc、现成堆分配器或要求先实现 Buddy System。
```
