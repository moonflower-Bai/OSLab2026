#!/usr/bin/env bash
set -eu

if ! command -v python3 >/dev/null 2>&1; then
    printf '%s\n' '[运行器][依赖] 错误：找不到 python3' \
        '汇总：单元测试=未运行；QEMU 集成测试=未运行；模型仿真测试=未运行'
    exit 1
fi

exec python3 -B "$(dirname "${BASH_SOURCE[0]}")/run_vmem_tests.py" "$@"
