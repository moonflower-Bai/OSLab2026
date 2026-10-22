## ADDED Requirements

### Requirement: Object allocation builds on physical pages

对象分配器 SHALL 在物理页初始化后提供 `kmalloc(size)` 和 `kfree(ptr)`，成功申请 SHALL 获得至少 size 字节且按 16 字节对齐、与其他存活对象不重叠的内核可访问空间。底层页获取与归还 MUST 使用现有公共物理页接口。

#### Scenario: Mixed small and large requests

- **WHEN** 调用者申请不同正长度，包括小于一页与大于一页的对象
- **THEN** 成功结果各自可以读写请求长度，互不覆盖，且可以仅凭返回指针归还

### Requirement: Slabs reuse object slots and return empty pages

小对象 SHALL 按大小缓存并共享底层页；释放后槽位 SHALL 可复用；所有对象归还后 SHALL 恢复本轮占用的底层页容量。

#### Scenario: A full slab becomes partial

- **WHEN** 某个 slab 已填满，调用者归还其中一个对象再申请同类对象
- **THEN** 原 slab 的空闲槽位可以再次使用，不因该申请额外取得物理页

#### Scenario: A workload releases every object

- **WHEN** 一组合法大小混合申请的所有成功对象均被归还
- **THEN** 空闲物理页数恢复到申请前

### Requirement: Allocation failures preserve live objects

零长度、不可能容纳的请求与大小运算溢出 SHALL 返回 NULL；页获取失败 SHALL 不泄漏页面或破坏现有对象。`kfree(NULL)` SHALL 无动作。当前版本 SHALL 声明仅支持无并发调用场景。

#### Scenario: Physical pages are exhausted

- **WHEN** 底层没有空闲页且请求需要新页
- **THEN** 申请返回 NULL，已有对象内容不变，释放已有对象和耗尽测试所持页面后容量恢复

### Requirement: Extension tests preserve supplied framework checks

新增测试 MUST 保留原框架检查与评分工具；报告 SHALL 提供分别用于实现与测试的两份提示词，并说明覆盖行为、边界与实际运行结果。

#### Scenario: Reviewing validation evidence

- **WHEN** 读者按报告执行已声明命令
- **THEN** 可以分别判断构建、物理页自检、对象自检和主机回归，未运行的管理器或最终评分不被记为已通过
