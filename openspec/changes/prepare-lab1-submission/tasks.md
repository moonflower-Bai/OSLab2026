# Tasks

## 1. 落实要求与个人提交材料入口

- [ ] 1.1 新增 lab1/docs/requirements.md，把 audit.md 中的课程来源、交付目录、仓库缺项和未核对题目整理成可审阅清单；核对每项都有出处或明确的待核对标记，不将建议分工写成最终分工。
- [ ] 1.2 新增 lab1/README.md，链接要求清单、验证工具和报告材料；根 README 只增加入口链接，并检查指导书、进度、通用规范的既有顺序不变。

## 2. 环境与启动验证工具

- [ ] 2.1 实现 lab1/scripts/check_env.sh，报告真实工具、版本、目标架构及 GDB RV64 支持；执行 bash -n 和实际检查，并用缺失编译器的临时命令搜索环境确认返回非零且列出缺项，不修改全局配置。
- [ ] 2.2 实现 lab1/scripts/verify_boot.py，以有限时长构建和 QEMU -kernel 启动观察横幅；运行 python3 lab1/scripts/verify_boot.py 必须自行终止、返回 0 并记录实际横幅，同时验证不存在的工具与未出现横幅的失败路径不会被标为通过。
- [ ] 2.3 在 lab1/README.md 说明标准验证命令、旧 loader/FW_DYNAMIC 问题和显式固件选项；按文档执行命令确认与实际结果一致，保存本次小体积文本证据，明确正常循环后主动结束运行的含义。

## 3. GDB 启动观察与答辩材料

- [ ] 3.1 实现 lab1/scripts/verify_debug.py，使用回环连接、本次 ELF 符号及有限时长进程管理；运行 python3 lab1/scripts/verify_debug.py 确认复位 PC、kern_entry、sp 与 bootstacktop 比较、kern_init 均实际观察到，否则据实记录未满足条件。
- [ ] 3.2 验证 GDB 连接失败和超时时返回非零并清理本次进程；检查不会终止其他调试会话，补充可复现的手工双终端说明和本次输出证据。
- [ ] 3.3 新增 lab1/docs/defense.md，将交叉编译、ELF/镜像、链接布局、启动交接、栈与尾调用、初始化、SBI 输出和调试证据逐项对应源码及实际观察；核对本次 edata/end 和反汇编，不把固定地址推广为所有环境规则。
- [ ] 3.4 新增 lab1/report/hews-validation.md，注明本人实现与验证、课程骨架、队友材料及尚缺截图/评分/完整题目；逐项与本次证据核对，不生成不存在的截图或个人模型使用记录。

## 4. 课程 Prompt 的有限例外

- [ ] 4.1 修改 AGENTS.md、prompts/README.md 与 openspec/config.yaml，允许当前章节 lab1/report/prompt.md 和根 report/prompt.md 作为用户明确提供的课程材料；审查规则仍禁止普通问答自动保存、敏感原文及其他 change 的证据迁移。
- [ ] 4.2 修改 scripts/check_governance.py，分别验证 OpenSpec 证据格式和课程格式；用人工审阅声明齐备的安全样例、非法路径、敏感模式及缺少声明的样例检查允许与拒绝结果，确保失败只报告位置、不改写文件。
- [ ] 4.3 文档说明课程 Prompt 必须由用户明确提供；用缺少输入的情况验证会报告缺项，且不会读取其他 change 或遗留会话补齐内容。

## 5. 课程交付导出工具

- [ ] 5.1 实现 scripts/export_lab1_submission.py，将当前章节和完整报告输入导出至仓库外新目录；用临时完整样例确认 code/、report/report.md、report/prompt.md、report/images/ 均存在，源码内容除导出行尾转换外一致且源文件未变。
- [ ] 5.2 用缺失报告、缺失引用截图、缺少 Prompt 审阅声明、报告占位和已有目标目录的样例验证非零退出与拒绝覆盖；确认失败不留下可误认为完整交付的结果，未复制本机工具、下载包、node_modules、obj/bin 或临时调试文件。
- [ ] 5.3 在 lab1/docs/requirements.md 写明导出命令、输入前提和最终分支操作边界，按临时样例复现；明确完整课程材料、真实截图、成员记录及正式题目审查仍须由小组补齐，导出工具不创建 commit、push、PR 或最终分支。

## 6. 集成验证与提交审查

- [ ] 6.1 从仓库根执行 make -C lab1，要求自行终止、退出 0 并生成 lab1/bin/kernel；随后运行仓库内启动与 GDB 验证，区分通过、失败和外部条件不足，不运行缺少脚本的 grade 目标。
- [ ] 6.2 执行 openspec validate prepare-lab1-submission --type change --strict、git diff --check 与治理检查，要求本 change 校验和适用门禁通过；若既有规格或外部条件阻碍检查，记录具体原因，不擅自归档其他 change。
- [ ] 6.3 审查个人提交文件清单与证据，确认没有工具包、生成二进制、猜测身份信息或改动队友分支；报告具体 diff 与结果，等待用户分别明确授权 Git commit、push/PR 和后续归档，不将任务实现完成等同于课程整体交付完成。
