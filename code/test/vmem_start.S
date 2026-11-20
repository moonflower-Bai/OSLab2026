    .section .text.entry, "ax"
    .globl _start
_start:
    csrci sstatus, 2
    csrw sie, zero
    la sp, __stack_top
    la t0, __bss_start
    la t1, __bss_end
1:
    bgeu t0, t1, 2f
    sd zero, 0(t0)
    addi t0, t0, 8
    j 1b
2:
    la t0, vmem_trap_entry
    csrw stvec, t0
    call vmem_guest_main
3:
    wfi
    j 3b

    .section .text.vmem_trap, "ax"
    .balign 4
    .globl vmem_trap_entry
vmem_trap_entry:
    addi sp, sp, -16
    sd ra, 0(sp)
    csrr a0, scause
    csrr a1, stval
    csrr a2, sepc
    call vmem_guest_trap
    csrw sepc, a0
    ld ra, 0(sp)
    addi sp, sp, 16
    sret

    .section .text.vmem_probes, "ax"
    .globl vmem_probe_load, vmem_load_pc, vmem_load_resume
vmem_probe_load:
vmem_load_pc:
    ld t0, 0(a0)
    sd t0, 0(a1)
vmem_load_resume:
    ret

    .globl vmem_probe_store, vmem_store_pc, vmem_store_resume
vmem_probe_store:
vmem_store_pc:
    sd a1, 0(a0)
vmem_store_resume:
    ret
