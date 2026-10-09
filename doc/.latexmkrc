# -*- mode: perl; -*-
# latexmk 配置：所有产物（PDF / 日志 / 中间文件）统一输出到 out/ 目录
#
# 说明：latexmk 只在“当前工作目录”和 $HOME 查找 .latexmkrc，不会向上级目录
# 查找。实际编译通常发生在 compile/<LabN>/ 里，此时生效的是 ~/.latexmkrc
# （内容与本文件一致）。本文件保持同样内容，用于在 /home/vscserver/latex
# 下直接编译时也能得到相同行为；两处修改请保持同步。

# 中文文档（ctexart）必须用 xelatex：latexmk 的 -pdf 默认调 pdflatex，必然失败。
# 命令行上的 -xelatex 会覆盖 $pdflatex，所以两个都设。
$xelatex  = 'xelatex -shell-escape %O %S';
$pdflatex = 'xelatex -shell-escape %O %S';

# 不写任何选项时也默认走 PDF 模式（否则 latexmk 会用 `latex` 出 dvi，ctexart 会报错）
$pdf_mode = 1;

# 结果文件 + 中间文件都进 out/
$out_dir = 'out';
$aux_dir = 'out';

# latexmk -c 时要清理的扩展名（含 biber 的 .bcf/.run.xml）
$clean_ext = 'aux fls log fdb_latexmk synctex.gz xdv bbl blg out toc lof lot nav snm vrb bcf run.xml';
