# Spec Delta

## Purpose

规定 Lab2 内核依据启动时提供的物理内存信息建立可分配页集合，并向内核提供连续物理页的 Best Fit 分配和回收行为。该契约用于检验保留区域不会被分配、成功和失败请求的统计一致，以及内存回收后的可重用性。

## ADDED Requirements

### Requirement: Physical memory initialization respects occupied regions

内核 SHALL 根据启动设备树报告的内存范围初始化物理页管理，使用 4 KiB 页粒度。固件、内核镜像、页描述信息占用的页和不完整边界页 MUST 保持不可分配；剩余完整页 SHALL 登记为可用物理页。

#### Scenario: Initialization with a valid memory range

- **WHEN** 内核完成初始化且设备树报告有效物理内存范围
- **THEN** 可用页数大于零且不超过该范围内的页数，物理地址与页描述之间的往返转换正确
- **AND** 内核末尾所在页及页描述信息所在页不能被分配

### Requirement: Allocation chooses a smallest sufficient contiguous block

对正整数页数的合法请求，分配器 SHALL 返回满足请求的最小空闲连续块中的所需页数；存在更大的可用块时仍 MUST 优先使用更小且足够的块。返回页 SHALL 与仍被占用的页互不重叠。

#### Scenario: Choosing between two sufficient blocks

- **WHEN** 空闲块分别包含一页和两页，调用方申请一页
- **THEN** 返回原一页块，原两页块仍可用于后续两页连续分配

#### Scenario: Splitting a larger block

- **WHEN** 最小足够块大于请求页数
- **THEN** 返回请求的连续页，剩余页仍可分配，空闲页数仅减少请求页数

### Requirement: Unsatisfied allocation preserves allocator state

没有足够大的连续空闲块时，分配器 SHALL 返回空指针，并 MUST 保持已有占用关系、空闲块和空闲页数不变。

#### Scenario: Free pages are fragmented

- **WHEN** 空闲页总数足够但所有连续空闲块均小于请求页数
- **THEN** 分配失败且不会消耗任何空闲页

### Requirement: Released adjacent ranges become reusable together

对已分配且尚未释放的合法连续页范围执行回收后，分配器 SHALL 恢复对应空闲页数，并将物理地址相邻的空闲范围合并为可连续分配的范围。

#### Scenario: A released range joins both neighbors

- **WHEN** 一段已分配页的左右相邻范围均空闲，调用方释放中间范围
- **THEN** 调用方可以一次分配这三个范围合计的连续页数

#### Scenario: Allocation and release restore the initial count

- **WHEN** 调用方完成一组合法分配并释放全部所得页
- **THEN** 空闲页总数恢复至操作前，原本可满足的连续分配仍可满足
