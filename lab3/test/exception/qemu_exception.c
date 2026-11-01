/*
 * QEMU 层检查。需要完整内核；exception.sh 不编译、不运行本文件。
 * 链接时使用 -Wl,--wrap=trap，使进入 trap 时能比较 tf 与 sp。
 * S 态不读取 medeleg，委托是否生效只看 ebreak 是否进入 S 态陷阱。
 */
#include <console.h>
#include <riscv.h>
#include <sbi.h>
#include <stdio.h>
#include <trap.h>

#define REQUIRE_SLOT(id, member, index)                                      \
    typedef char slot_##id[(__builtin_offsetof(struct trapframe, member) ==   \
                            (index) * sizeof(uintptr_t))                      \
                               ? 1                                            \
                               : -1]

REQUIRE_SLOT(zero, gpr.zero, 0);
REQUIRE_SLOT(ra, gpr.ra, 1);
REQUIRE_SLOT(sp, gpr.sp, 2);
REQUIRE_SLOT(gp, gpr.gp, 3);
REQUIRE_SLOT(tp, gpr.tp, 4);
REQUIRE_SLOT(t0, gpr.t0, 5);
REQUIRE_SLOT(t1, gpr.t1, 6);
REQUIRE_SLOT(t2, gpr.t2, 7);
REQUIRE_SLOT(s0, gpr.s0, 8);
REQUIRE_SLOT(s1, gpr.s1, 9);
REQUIRE_SLOT(a0, gpr.a0, 10);
REQUIRE_SLOT(a1, gpr.a1, 11);
REQUIRE_SLOT(a2, gpr.a2, 12);
REQUIRE_SLOT(a3, gpr.a3, 13);
REQUIRE_SLOT(a4, gpr.a4, 14);
REQUIRE_SLOT(a5, gpr.a5, 15);
REQUIRE_SLOT(a6, gpr.a6, 16);
REQUIRE_SLOT(a7, gpr.a7, 17);
REQUIRE_SLOT(s2, gpr.s2, 18);
REQUIRE_SLOT(s3, gpr.s3, 19);
REQUIRE_SLOT(s4, gpr.s4, 20);
REQUIRE_SLOT(s5, gpr.s5, 21);
REQUIRE_SLOT(s6, gpr.s6, 22);
REQUIRE_SLOT(s7, gpr.s7, 23);
REQUIRE_SLOT(s8, gpr.s8, 24);
REQUIRE_SLOT(s9, gpr.s9, 25);
REQUIRE_SLOT(s10, gpr.s10, 26);
REQUIRE_SLOT(s11, gpr.s11, 27);
REQUIRE_SLOT(t3, gpr.t3, 28);
REQUIRE_SLOT(t4, gpr.t4, 29);
REQUIRE_SLOT(t5, gpr.t5, 30);
REQUIRE_SLOT(t6, gpr.t6, 31);
REQUIRE_SLOT(status, status, 32);
REQUIRE_SLOT(epc, epc, 33);
REQUIRE_SLOT(badvaddr, badvaddr, 34);
REQUIRE_SLOT(cause, cause, 35);

typedef char slot_count[(sizeof(struct trapframe) == 36 * sizeof(uintptr_t)) ? 1 : -1];

uintptr_t qemu_sp_before;
uintptr_t qemu_sp_after;
uintptr_t qemu_regs[32];
uintptr_t qemu_landed;
uintptr_t qemu_sscratch_after;
uintptr_t qemu_trap_seen;
uintptr_t qemu_tf;
uintptr_t qemu_entry_sp;
uintptr_t qemu_entry_sscratch;
uintptr_t qemu_entry_scause;
uintptr_t qemu_entry_sstatus;
uintptr_t qemu_tf_words[36];

extern void qemu_restore_probe(void);

void exception_qemu_test(void);

void __wrap_kern_init(void) { exception_qemu_test(); }

static void report(int ok, const char *name, unsigned long actual) {
    if (ok) {
        cprintf("通过 %s\n", name);
    } else {
        cprintf("错误 %s: 0x%lx\n", name, actual);
    }
}

static void report_reg(int index) {
    char name[4];
    int ok;
    unsigned long actual;

    name[0] = 'x';
    if (index >= 10) {
        name[1] = (char)('0' + index / 10);
        name[2] = (char)('0' + index % 10);
        name[3] = '\0';
    } else {
        name[1] = (char)('0' + index);
        name[2] = '\0';
    }
    if (index == 0) {
        ok = qemu_trap_seen != 0 && qemu_regs[0] == 0;
        actual = (unsigned long)qemu_regs[0];
    } else if (index == 2) {
        ok = qemu_landed == 1 && qemu_sp_after == qemu_sp_before && qemu_regs[2] == qemu_sp_before;
        actual = (unsigned long)qemu_sp_after;
    } else {
        ok = qemu_landed == 1 && qemu_regs[index] == (uintptr_t)index;
        actual = (unsigned long)qemu_regs[index];
    }
    report(ok, name, actual);
}

static void report_tf_reg(int index) {
    char name[8];
    unsigned long actual = (unsigned long)qemu_tf_words[index];
    int ok = qemu_trap_seen != 0;

    name[0] = 't';
    name[1] = 'f';
    name[2] = '-';
    name[3] = 'x';
    if (index >= 10) {
        name[4] = (char)('0' + index / 10);
        name[5] = (char)('0' + index % 10);
        name[6] = '\0';
    } else {
        name[4] = (char)('0' + index);
        name[5] = '\0';
    }
    if (ok && index == 2) {
        ok = qemu_tf_words[index] == qemu_sp_before;
    } else if (ok && index != 0) {
        ok = qemu_tf_words[index] == (uintptr_t)index;
    } else if (ok) {
        ok = qemu_tf_words[index] == 0;
    }
    report(ok, name, actual);
}

void exception_qemu_test(void) {
    int i;

    cons_init();
    idt_init();
    qemu_restore_probe();

    report(qemu_trap_seen != 0 && qemu_tf == qemu_entry_sp, "trap指针",
           (unsigned long)(qemu_trap_seen == 0 ? qemu_trap_seen : qemu_tf));
    report(qemu_trap_seen != 0 && qemu_entry_sscratch == 0, "进入sscratch",
           (unsigned long)qemu_entry_sscratch);
    report(qemu_landed == 1 && qemu_sscratch_after == 0, "返回sscratch",
           (unsigned long)qemu_sscratch_after);
    report(qemu_trap_seen != 0 && qemu_entry_scause == 3, "scause", (unsigned long)qemu_entry_scause);
    report(qemu_trap_seen != 0 && (qemu_entry_sstatus & SSTATUS_SPP) != 0, "sstatus",
           (unsigned long)qemu_entry_sstatus);
    report(qemu_landed == 1, "下一条指令", (unsigned long)qemu_landed);

    for (i = 0; i <= 31; i++) {
        report_reg(i);
    }
    for (i = 0; i <= 31; i++) {
        report_tf_reg(i);
    }
    report(qemu_trap_seen != 0 && qemu_tf_words[35] == 3, "tf-cause", (unsigned long)qemu_tf_words[35]);
    report(qemu_trap_seen != 0 && (qemu_tf_words[32] & SSTATUS_SPP) != 0, "tf-spp",
           (unsigned long)qemu_tf_words[32]);
    sbi_shutdown();
}
