# Tasks

## 1. 页操作实现

- [x] 1.1 实现 `page_insert()`；核对 diff 中失败返回先于数据页引用变更、同页更新不改引用，以及换页写入新叶项后先失效再减少旧页引用。
- [x] 1.2 实现 `page_remove()` 和本地翻译失效顺序；核对 diff 中空路径/无效项直接返回，有效项清零后先失效再按引用计数回收，失效指令有编译器内存屏障。

## 2. 验证与交付

- [x] 2.1 使用 `bash lab4/test/run_vmem_tests.sh` 验证单元、QEMU 集成和模型仿真，记录各层实测结果及独立于验收的已知构建阻塞；使用 `git diff --check` 核对并确认只改动约定范围，不读取 `lab4/tools/`。

外部前提：OpenSpec CLI、Bash、Python 3、64 位宿主 C 工具链、RV64 GCC 和具有 `virt`、TCG、OpenSBI 的 QEMU。

## Verification

- `bash lab4/test/run_vmem_tests.sh` 最终退出 `0`。单元测试 9/9 通过，包含两级分配失败及重试。QEMU 集成测试 2/2 通过，从 `VMEM_BOOT` 到 `VMEM_DONE PASS`，观察到降权后的写缺页（`scause=15`）及撤销后的读缺页（`scause=13`）。模型 4 个固定种子各完成 256 步，共 1024 步通过。
- RV64 GCC `-O2` 汇编中，新建、替换、清除叶项的 PTE 写入均在相应 `sfence.vma` 之前；替换和删除的旧页引用减少与回收代码位于失效之后。
- 既有构建阻塞另行用 RV64 GCC `-fsyntax-only` 复现：`lab4/kern/trap/trap.c:102` 和 `lab4/kern/trap/trap.c:107` 调用缺失的 `interrupt_handler()` 和 `exception_handler()`；`lab4/kern/mm/kmalloc.c:92` 将整数传给要求 `void *` 的 `kva2page()`。未修改这两个模块，未调用主构建或评分入口，未读取 `lab4/tools/`。
