/*
 * QEMU 层保存/恢复探针。exception.sh 不链接、不运行本文件。
 * 链入完整内核时需要 -Wl,--wrap=trap。
 * 不读取 medeleg。
 */
    .section .text
    .align 2

    .macro save_orig n
    sd x\n, (\n * 8)(sp)
    .endm

    .macro dump_sent n
    sd x\n, (256 + \n * 8)(sp)
    .endm

    .macro copy_sent n
    ld t1, (256 + \n * 8)(sp)
    sd t1, (\n * 8)(t0)
    .endm

    .macro load_orig n
    ld x\n, (\n * 8)(sp)
    .endm

    .macro set_sent n
    li x\n, \n
    .endm

    .globl qemu_restore_probe
qemu_restore_probe:
    addi sp, sp, -512
    save_orig 1
    save_orig 2
    save_orig 3
    save_orig 4
    save_orig 5
    save_orig 6
    save_orig 7
    save_orig 8
    save_orig 9
    save_orig 10
    save_orig 11
    save_orig 12
    save_orig 13
    save_orig 14
    save_orig 15
    save_orig 16
    save_orig 17
    save_orig 18
    save_orig 19
    save_orig 20
    save_orig 21
    save_orig 22
    save_orig 23
    save_orig 24
    save_orig 25
    save_orig 26
    save_orig 27
    save_orig 28
    save_orig 29
    save_orig 30
    save_orig 31
    la t0, qemu_sp_before
    sd sp, 0(t0)

    set_sent 1
    set_sent 3
    set_sent 4
    set_sent 5
    set_sent 6
    set_sent 7
    set_sent 8
    set_sent 9
    set_sent 10
    set_sent 11
    set_sent 12
    set_sent 13
    set_sent 14
    set_sent 15
    set_sent 16
    set_sent 17
    set_sent 18
    set_sent 19
    set_sent 20
    set_sent 21
    set_sent 22
    set_sent 23
    set_sent 24
    set_sent 25
    set_sent 26
    set_sent 27
    set_sent 28
    set_sent 29
    set_sent 30
    set_sent 31

    .option push
    .option norvc
    ebreak
    .option pop

    dump_sent 1
    dump_sent 2
    dump_sent 3
    dump_sent 4
    dump_sent 5
    dump_sent 6
    dump_sent 7
    dump_sent 8
    dump_sent 9
    dump_sent 10
    dump_sent 11
    dump_sent 12
    dump_sent 13
    dump_sent 14
    dump_sent 15
    dump_sent 16
    dump_sent 17
    dump_sent 18
    dump_sent 19
    dump_sent 20
    dump_sent 21
    dump_sent 22
    dump_sent 23
    dump_sent 24
    dump_sent 25
    dump_sent 26
    dump_sent 27
    dump_sent 28
    dump_sent 29
    dump_sent 30
    dump_sent 31

    la t0, qemu_regs
    copy_sent 1
    copy_sent 2
    copy_sent 3
    copy_sent 4
    copy_sent 5
    copy_sent 6
    copy_sent 7
    copy_sent 8
    copy_sent 9
    copy_sent 10
    copy_sent 11
    copy_sent 12
    copy_sent 13
    copy_sent 14
    copy_sent 15
    copy_sent 16
    copy_sent 17
    copy_sent 18
    copy_sent 19
    copy_sent 20
    copy_sent 21
    copy_sent 22
    copy_sent 23
    copy_sent 24
    copy_sent 25
    copy_sent 26
    copy_sent 27
    copy_sent 28
    copy_sent 29
    copy_sent 30
    copy_sent 31

    csrr t1, sscratch
    la t0, qemu_sscratch_after
    sd t1, 0(t0)
    la t0, qemu_sp_after
    sd sp, 0(t0)
    la t0, qemu_landed
    li t1, 1
    sd t1, 0(t0)

    load_orig 1
    load_orig 3
    load_orig 4
    load_orig 6
    load_orig 7
    load_orig 8
    load_orig 9
    load_orig 10
    load_orig 11
    load_orig 12
    load_orig 13
    load_orig 14
    load_orig 15
    load_orig 16
    load_orig 17
    load_orig 18
    load_orig 19
    load_orig 20
    load_orig 21
    load_orig 22
    load_orig 23
    load_orig 24
    load_orig 25
    load_orig 26
    load_orig 27
    load_orig 28
    load_orig 29
    load_orig 30
    load_orig 31
    load_orig 5
    addi sp, sp, 512
    ret

    .globl __wrap_trap
    .align 2
__wrap_trap:
    la t0, qemu_trap_seen
    li t1, 1
    sd t1, 0(t0)
    la t0, qemu_tf
    sd a0, 0(t0)
    ld t1, 0(a0)
    la t0, qemu_regs
    sd t1, 0(t0)
    la t0, qemu_entry_sp
    sd sp, 0(t0)
    csrr t1, sscratch
    la t0, qemu_entry_sscratch
    sd t1, 0(t0)
    csrr t1, scause
    la t0, qemu_entry_scause
    sd t1, 0(t0)
    csrr t1, sstatus
    la t0, qemu_entry_sstatus
    sd t1, 0(t0)
    la t0, qemu_tf_words
    mv t1, a0
    li t2, 36
.Lcopy_tf:
    ld t3, 0(t1)
    sd t3, 0(t0)
    addi t1, t1, 8
    addi t0, t0, 8
    addi t2, t2, -1
    bnez t2, .Lcopy_tf
    addi sp, sp, -16
    sd ra, 8(sp)
    la t0, qemu_sp_before
    ld t0, 0(t0)
    ld gp, 24(t0)
    ld tp, 32(t0)
    jal __real_trap
    li gp, 3
    li tp, 4
    ld ra, 8(sp)
    addi sp, sp, 16
    ret
