# Tasks

## 1. 核对原始证据

- [x] 1.1 逐张查看 12 张 PNG，并阅读 3 份日志；核对练习 1 的 SP/RA 变化和练习 2 的复位、固件交接结果，确认旧日志初始 PC 为 0x8020003a，形成逐图说明。
- [x] 1.2 比较实验副本与仓库对应的 21 个源码及构建文件；读取 ELF 符号及工具版本，确认源码字节一致、入口及栈地址与截图一致。

## 2. 导入与编写说明

- [x] 2.1 导入 12 张原图和 3 份日志，将旧会话放入 diagnostics/；生成 evidence-manifest.json，通过逐项 SHA-256 与源文件比较确认 15 份材料未改变。
- [x] 2.2 新增说明 README，包含逐图索引、两次练习的操作与目的、六条复位指令、mret 交接、观察点结论、复现命令和报告材料用途；检查所有本地链接存在且覆盖全部 12 张截图。
- [x] 2.3 根 README 增加材料入口；确认指导书、进度和通用规范顺序不变，根说明未新增单章运行命令。

## 3. 集成与提交审查

- [x] 3.1 执行 make -C lab1、openspec validate document-lab1-exercise-evidence --type change --strict、python3 scripts/check_governance.py --self-test 和 git diff --check；要求适用检查通过，构建自行结束且生成 kernel，不执行缺少脚本的 grade。
- [x] 3.2 仅暂存根 README、lab1/report/record_Lab1/ 和本 change；用 git diff --cached --name-only、git diff --cached --check 与文件哈希清单核对范围，确认没有内核改动、工具包、生成镜像和已有未跟踪提案。

外部前提：本机前置目录内的原始材料、已安装的 RISC-V 工具链和 OpenSpec。材料校验使用 evidence-manifest.json 的路径及 SHA-256，复现命令中的 FW_JUMP 路径须替换为实际安装位置。

用户本次请求已明确授权说明整理和一个本地 commit；完成以上任务后创建该 commit。push 由用户执行，规格归档不在本次范围内。
