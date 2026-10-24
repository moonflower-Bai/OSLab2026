## ADDED Requirements

### Requirement: Official Lab3 skeleton is imported unchanged
仓库 MUST 在 `lab3/` 下包含官方 `lab3.zip` 解压出的 `lab3_new` 目录的全部源文件，内容 MUST 与官方发布一致，练习位置 MUST 保持未填写。

#### Scenario: Compare imported files with the official archive
- **WHEN** 将 `lab3/` 下的源文件逐一与官方压缩包中 `lab3_new` 的对应文件比较（排除 `lab3/AGENTS.md`）
- **THEN** 两者文件集合相同，且内容逐字节一致

#### Scenario: Exercise markers are preserved
- **WHEN** 在 `lab3/kern` 中搜索 `lab3` 与 `YOUR CODE` 的注释标记
- **THEN** 官方骨架中的全部标记仍然存在，且没有被替换为学号

### Requirement: Build artifacts and OS metadata are excluded
导入 MUST NOT 包含 `bin/`、`obj/`、`.DS_Store` 或 `__MACOSX` 等构建产物与系统元数据。

#### Scenario: List tracked files under lab3
- **WHEN** 列出 `lab3/` 下被版本控制的文件
- **THEN** 其中没有构建产物或系统元数据文件

### Requirement: Current chapter points to lab3
`lab-status.json` 的 `current` MUST 为 `lab3`，`frozen` MUST 为空数组，使 `Dev/Lab3/*` 分支通过章节一致性检查。

#### Scenario: Run governance check on a Lab3 branch
- **WHEN** 在 `Dev/Lab3/<user>/<branch>` 分支上运行 `python3 scripts/check_governance.py`
- **THEN** 检查通过，不报告章节与分支名不一致

### Requirement: Unfinished skeleton state is documented
`lab3/AGENTS.md` MUST 说明骨架在练习完成前无法通过编译，并列出构建与验证命令。

#### Scenario: A developer reads the chapter guide before starting
- **WHEN** 开发者阅读 `lab3/AGENTS.md`
- **THEN** 能得知导入后的 `make -C lab3` 在练习完成前失败属预期，以及练习完成后应执行的验证命令
