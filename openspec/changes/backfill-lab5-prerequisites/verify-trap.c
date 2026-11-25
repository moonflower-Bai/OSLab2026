#include <assert.h>
#include <clock.h>
#include <pmm.h>
#include <proc.h>
#include <riscv.h>
#include <sbi.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>

extern void forkrets(struct trapframe *tf) __attribute__((noreturn));
extern void lab5_kernel_trap(struct trapframe *tf);
extern const unsigned char probe_start[], probe_end[], probe_ecall1[], probe_ecall2[];

asm(".pushsection .rodata\n"
    ".balign 4\n"
    ".option push\n"
    ".option norvc\n"
    "probe_start: sd a0, -8(sp)\n"
    "ld a0, -8(sp)\n"
    "probe_ecall1: ecall\n"
    "probe_ecall2: ecall\n"
    "probe_end:\n"
    ".option pop\n"
    ".popsection");

static struct trapframe *user_frame;
static struct pushregs expected_regs;
static uintptr_t kernel_stack_top;
static unsigned int user_traps, nested_traps;
static bool worker_ran;

static int worker(void *arg)
{
    worker_ran = 1;
    current->state = PROC_SLEEPING;
    schedule();
    panic("sleeping test thread resumed");
}

void trap(struct trapframe *tf)
{
    if (tf->cause == CAUSE_BREAKPOINT && trap_in_kernel(tf))
    {
        assert(read_csr(sscratch) == 0);
        assert((uintptr_t)(tf + 1) == tf->gpr.sp);
        assert(tf->gpr.sp < kernel_stack_top);
        nested_traps++;
        lab5_kernel_trap(tf);
        return;
    }

    if (tf->cause != CAUSE_USER_ECALL)
    {
        lab5_kernel_trap(tf);
        return;
    }

    assert(!trap_in_kernel(tf));
    assert((read_csr(sstatus) & SSTATUS_SIE) == 0);
    assert(read_csr(sscratch) == 0 && tf == user_frame);
    assert((uintptr_t)(tf + 1) == kernel_stack_top);
    assert(memcmp(&expected_regs, &tf->gpr, sizeof(expected_regs)) == 0);
    uintptr_t probe_offset = user_traps == 0 ? probe_ecall1 - probe_start
                                            : probe_ecall2 - probe_start;
    assert(tf->epc == UTEXT + probe_offset);
    struct trapframe before = *tf;
    uintptr_t restored_t0;
    asm volatile("li t0, 0x51c0; ebreak; mv %0, t0"
                 : "=r"(restored_t0) : : "t0", "memory");
    assert(restored_t0 == 0x51c0);
    assert(read_csr(sscratch) == 0);
    assert(memcmp(tf, &before, sizeof(before)) == 0);
    assert(++user_traps == nested_traps - 1);
    if (user_traps == 2)
    {
        cprintf("Lab5 probe passed: timer, scheduling, S-mode and repeated U-mode trap return\n");
        sbi_shutdown();
        panic("SBI shutdown returned");
    }
    tf->epc += 4;
}

int lab5_verify_trap(void)
{
    int pid = kernel_thread(worker, NULL, 0);
    assert(pid > 0);
    schedule();
    assert(worker_ran && current == initproc);
    assert(find_proc(pid)->state == PROC_SLEEPING);

    size_t start_ticks = ticks;
    while (ticks - start_ticks < 105)
        asm volatile("wfi");

    kernel_stack_top = current->kstack + KSTACKSIZE;
    asm volatile("ebreak" ::: "memory");
    assert(nested_traps == 1 && read_csr(sscratch) == 0);

    struct Page *code = pgdir_alloc_page(boot_pgdir_va, UTEXT, PTE_U | PTE_R | PTE_X);
    assert(code != NULL);
    assert(pgdir_alloc_page(boot_pgdir_va, USTACKTOP - PGSIZE,
                            PTE_U | PTE_R | PTE_W) != NULL);
    memcpy(page2kva(code), probe_start, probe_end - probe_start);
    asm volatile("fence.i" ::: "memory");

    uintptr_t regs[32] = {0};
    for (unsigned int i = 1; i < 32; i++)
        regs[i] = 0xabc00000 + i;
    regs[2] = USTACKTOP;
    memcpy(&expected_regs, regs, sizeof(regs));

    // Reuse the thread's initial trapframe at the top of its kernel stack.
    clear_csr(sstatus, SSTATUS_SIE);
    user_frame = (struct trapframe *)kernel_stack_top - 1;
    user_frame->gpr = expected_regs;
    user_frame->epc = UTEXT;
    user_frame->status = (read_csr(sstatus) & ~SSTATUS_SPP) | SSTATUS_SPIE;
    user_frame->cause = 0;
    user_frame->tval = 0;
    forkrets(user_frame);
}
