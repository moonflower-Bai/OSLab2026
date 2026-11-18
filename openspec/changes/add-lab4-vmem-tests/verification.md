# 实测结果

## 仓库真实实现

在仓库根运行 `bash lab4/test/run_vmem_tests.sh`，依次尝试三层构建。实际 `pmm.c` 尚未定义 `page_insert`、`page_remove`，宿主和 RV64 链接器均报告这两个未定义符号。三层均标为「未运行」，出口为 `1`；默认运行未启动 QEMU，也未执行任何断言。

## 测试临时副本

在 `lab4/test/` 下建临时副本，仅复制 `lab4/kern/`、`lab4/libs/` 和本次测试文件。在该副本的 `pmm.c` 加入仅用于验收的正向对照，仍使用实际 `get_pte()`、物理页接口和 `tlb_invalidate()`。从临时副本根运行同一入口：9 项单元测试、2 项 QEMU 真实访存测试和 4 组各 256 步的模型测试全部通过，出口为 `0`。实际观察到 `VMEM_BOOT`、`VMEM_DONE PASS`、store page fault（`scause=15`）和 load page fault（`scause=13`），两个故障的 `stval` 均为 `0x400ea000`。

每个变体独立从正向对照开始，并在同一入口报告错误、返回 `1`：

| 临时改动 | 检出的用例与实际观察 |
| --- | --- |
| 原址原页更新权限时额外增加一次引用 | 单元 `same-page-permission`：实际 `ref=2`，期望 `1` |
| 末次删除时减引用但不回收页 | 单元 `remove-last-repeat`：实际 `ref=0 owned=1 frees=0`，期望 `ref=0 owned=0 frees=1` |
| 换页时跳过 TLB 失效，保留初次映射和同页更新的失效 | 四个模型种子均在第 6 步检出旧翻译（`root=0 va=0x1ff000`）；QEMU 换页后同址首次读取仍读到 A 而非 B |

另在该副本分别省略降权、撤销的失效，模型各自在第 7 步、第 8 步检出，QEMU 分别观测到未产生预期的 store/load page fault。指定不存在的 `QEMU` 可执行文件及极短的 QEMU 超时均验证了返回 `1`、报告原因、继续运行模型层。所有对照和变体仅存在于测试临时目录，验证后清理，不交付替代实现。
