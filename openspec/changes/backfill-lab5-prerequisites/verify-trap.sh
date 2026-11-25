#!/bin/sh
set -eu

change_dir=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
repo=$(CDPATH='' cd -- "$change_dir/../../.." && pwd)
scratch=$(mktemp -d /tmp/oslab-lab5-verify.XXXXXX)
trap 'rm -rf -- "$scratch"' EXIT
tar -C "$repo/lab5" --exclude=obj --exclude=bin -cf "$scratch/source.tar" .
tar -C "$scratch" -xf "$scratch/source.tar"
cp "$change_dir/verify-trap.c" "$scratch/kern/init/verify_trap.c"
# Keep the real trap dispatcher for timer interrupts and kernel breakpoints.
sed -i 's/^void trap(struct trapframe \*tf)/void lab5_kernel_trap(struct trapframe *tf)/' "$scratch/kern/trap/trap.c"
sed -i '/^init_main(void \*arg)/,+1!b; /^{/a\    extern int lab5_verify_trap(void);\n    return lab5_verify_trap();' "$scratch/kern/process/proc.c"

# The Lab5 syscall exercise is independent of the trap-return path.
make -C "$scratch" -j2 bin/ucore.img \
    KSRCDIR='kern/init kern/libs kern/debug kern/driver kern/trap kern/mm kern/sync kern/process kern/schedule' \
    USRCDIR=
timeout 12s "${QEMU:-qemu-system-riscv64}" -machine virt -m 128M -nographic \
    -bios default -kernel "$scratch/bin/ucore.img" > "$scratch/qemu.log" 2>&1 || {
    cat "$scratch/qemu.log"
    exit 1
}
cat "$scratch/qemu.log"
rg -F 'check_alloc_page() succeeded!' "$scratch/qemu.log"
rg -F 'check_pgdir() succeeded!' "$scratch/qemu.log"
rg -F 'check_boot_pgdir() succeeded!' "$scratch/qemu.log"
rg -F 'check_vmm() succeeded.' "$scratch/qemu.log"
rg -F 'Lab5 probe passed:' "$scratch/qemu.log"
