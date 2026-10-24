## 1. Import

- [x] 1.1 下载 `http://oslab.mobisys.top/lab2026/lab3.zip` 到独立目录并解压（外部前提：网络可访问该站点）
- [x] 1.2 将 `lab3_new` 内容复制到 `lab3/`，排除 `.DS_Store`、`__MACOSX`、`bin`、`obj`
- [x] 1.3 新增 `lab3/AGENTS.md`，说明工具链、验证命令及骨架未完成时无法编译
- [x] 1.4 将 `lab-status.json` 改为 `current: lab3`、`frozen: []`

## 2. Verification

- [x] 2.1 `diff -r` 比较 `lab3/`（排除 `AGENTS.md`）与解压出的 `lab3_new`，应无差异
- [x] 2.2 `grep -rn "YOUR CODE" lab3/kern` 仍能找到官方标记
- [x] 2.3 `git ls-files lab3 | grep -E "(^|/)(bin|obj|\.DS_Store|__MACOSX)(/|$)"` 无输出
- [x] 2.4 `python3 scripts/check_governance.py` 通过
- [x] 2.5 `openspec validate import-lab3-code --strict` 通过
