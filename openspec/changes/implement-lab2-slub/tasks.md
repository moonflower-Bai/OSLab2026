# Tasks

## 1. Scope and architecture

- [x] 1.1 读取当前章节规则及公共页接口，确认本次只实现 SLUB 扩展。
- [x] 1.2 更新远端引用，从 `7074bb6` 建立合法个人分支并保留已有未提交文档。
- [x] 1.3 确定对象缓存、部分空闲 slab、归还及失败契约，准备实现提示词。

## 2. Implementation

- [x] 2.1 从零实现 SLUB 对象分配层并在页初始化后接入。
- [x] 2.2 编写独立内核测试及主机测试，不修改原框架自检。
- [x] 2.3 提供可复现的主机 sanitizer 测试入口与两份提示词。
- [x] 2.4 定位回归发现的 First Fit 尾插缺陷，补两处 `break` 和定向检查。

## 3. Verification and report

- [x] 3.1 `source ../Lab1前置准备/activate.sh` 后 `make -C lab2 clean`、`make -C lab2` 成功。
- [x] 3.2 128 MiB 和 64 MiB 下运行限时 QEMU，必须同时有物理页及 SLUB 成功标记，无 panic；124 仅记录为外部超时。
- [x] 3.3 主机回归和非法调用检查通过，ASan/UBSan 无错误；验证失败后修复并重跑相关检查。
- [x] 3.4 核对原测试逐字节未改；运行 OpenSpec 严格校验、治理及 diff 检查。
- [x] 3.5 报告记录真实输出、测试理由、能力边界和可审阅提交分组。
- [x] 3.6 清理后运行原 `make test`，同时完成当前 First Fit 与 SLUB 自检，不把当前结果记成 Best Fit 六阶段计分。
- [x] 3.7 对三组代码提交的暂存快照分别独立构建，并从测试快照复验主机与 128/64 MiB QEMU；文档组代码哈希与已验证测试组一致。

外部依赖为本机已有 RISC-V GCC/Binutils、QEMU/OpenSBI、GCC、Python 3 与 OpenSpec。验证命令如下，均在仓库根目录执行：

```bash
source ../Lab1前置准备/activate.sh
make -C lab2 clean
make -C lab2
timeout 15s make -C lab2 qemu
timeout 15s make -C lab2 qemu QEMU='qemu-system-riscv64 -m 64M'
make -C lab2 slub-host-test
make -C lab2 clean
timeout 20s make -C lab2 test
openspec validate implement-lab2-slub --strict
python3 -B scripts/check_governance.py --self-test --target Dev/Lab2/main
git diff --check
```

128/64 MiB 自检后分别恢复 31927/15703 页；主机 2000＋20000 次操作与 7 类隔离 panic 检查通过。原检查函数及评分工具逐字节未改；详情与真实日志见 `lab2/report/slub.md`。2026-10-22 用户已授权测试通过后 commit 并 push，按 design 和报告的四组结构执行；不归档或合并 PR。
