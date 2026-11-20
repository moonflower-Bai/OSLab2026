#!/bin/sh
set -eu

cd "$(dirname "$0")/../.."
test_bin=$(mktemp "${TMPDIR:-/tmp}/lab4-process-test.XXXXXX")
trap 'rm -f "$test_bin"' EXIT HUP INT TERM

"${RISCV_LINUX_CC:-riscv64-linux-gnu-gcc}" -static -O2 -std=gnu99 \
    -fno-builtin -Wno-unused -Itests/process-init -Ilibs -Ikern/debug \
    -Ikern/driver -Ikern/trap -Ikern/mm -Ikern/process -Ikern/schedule \
    tests/process-init/test.c libs/hash.c libs/string.c -o "$test_bin"
"${QEMU_RISCV64:-qemu-riscv64}" "$test_bin"
