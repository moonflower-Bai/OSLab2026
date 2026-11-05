#include <riscv.h>
#include <trap.h>

#include "host_runtime.h"

void __alltraps(void) {}

void interrupt_handler(struct trapframe *tf) {
    (void)tf;
    host_note_interrupt();
}

static void call_trap(void *frame) { trap((struct trapframe *)frame); }

static void join(char *dst, const char *a, const char *b) { host_join(dst, 96, a, b); }

static void name_cause(char *dst, const char *prefix, unsigned cause) {
    host_name_cause(dst, 96, prefix, cause);
}

static void check_marker_addr(const char *text, const char *marker, const char *banned,
                              uintptr_t epc, const char *name) {
    unsigned long long got = 0;

    if (host_has(text, banned)) {
        host_fail_text(name, text);
        return;
    }
    if (!host_has(text, marker) || !host_number_after(text, marker, &got)) {
        host_fail_text(name, text);
        return;
    }
    if (got != epc) {
        host_fail_hex(name, got);
        return;
    }
    host_pass(name);
}

static void check_cause_line(const char *text, uintptr_t cause, const char *name) {
    const char *key = "cause    0x";
    unsigned long long got = 0;

    if (!host_has(text, key) || !host_hex_after(text, key, &got)) {
        host_fail_text(name, text);
        return;
    }
    if (got != cause) {
        host_fail_hex(name, got);
        return;
    }
    host_pass(name);
}

static void check_exclusive(const char *text, const char *name) {
    if (host_has(text, "Breakpoint at") || host_has(text, "Illegal instruction at")) {
        host_fail_text(name, text);
        return;
    }
    host_pass(name);
}

static void check_panic(int how, int want, const char *name) {
    int called = (how == HOST_PANIC) || host_panic_called();

    if (want) {
        if (called) {
            host_pass(name);
        } else {
            host_fail_hex(name, 0);
        }
        return;
    }
    if (!called) {
        host_pass(name);
        return;
    }
    host_fail_text(name, host_panic_message());
}

static void check_moved(const char *tag, uintptr_t actual, uintptr_t previous, unsigned delta) {
    char name[96];

    join(name, tag, "epc");
    if (actual != previous + delta) {
        host_fail_hex(name, actual);
        return;
    }
    host_pass(name);
}

static void check_fields(const char *tag, const struct trapframe *before, const struct trapframe *after) {
    int bad = 0;
    char name[96];

#define SAME(field, label)                                          \
    do {                                                            \
        join(name, tag, label);                                     \
        if ((after)->field != (before)->field) {                    \
            host_fail_hex(name, (unsigned long long)(after)->field); \
            bad = 1;                                                \
        }                                                           \
    } while (0)

    SAME(gpr.zero, "gpr.zero");
    SAME(gpr.ra, "gpr.ra");
    SAME(gpr.sp, "gpr.sp");
    SAME(gpr.gp, "gpr.gp");
    SAME(gpr.tp, "gpr.tp");
    SAME(gpr.t0, "gpr.t0");
    SAME(gpr.t1, "gpr.t1");
    SAME(gpr.t2, "gpr.t2");
    SAME(gpr.s0, "gpr.s0");
    SAME(gpr.s1, "gpr.s1");
    SAME(gpr.a0, "gpr.a0");
    SAME(gpr.a1, "gpr.a1");
    SAME(gpr.a2, "gpr.a2");
    SAME(gpr.a3, "gpr.a3");
    SAME(gpr.a4, "gpr.a4");
    SAME(gpr.a5, "gpr.a5");
    SAME(gpr.a6, "gpr.a6");
    SAME(gpr.a7, "gpr.a7");
    SAME(gpr.s2, "gpr.s2");
    SAME(gpr.s3, "gpr.s3");
    SAME(gpr.s4, "gpr.s4");
    SAME(gpr.s5, "gpr.s5");
    SAME(gpr.s6, "gpr.s6");
    SAME(gpr.s7, "gpr.s7");
    SAME(gpr.s8, "gpr.s8");
    SAME(gpr.s9, "gpr.s9");
    SAME(gpr.s10, "gpr.s10");
    SAME(gpr.s11, "gpr.s11");
    SAME(gpr.t3, "gpr.t3");
    SAME(gpr.t4, "gpr.t4");
    SAME(gpr.t5, "gpr.t5");
    SAME(gpr.t6, "gpr.t6");
    SAME(status, "status");
    SAME(badvaddr, "badvaddr");
    SAME(cause, "cause");
#undef SAME

    if (!bad) {
        join(name, tag, "字段");
        host_pass(name);
    }
}

static void fill_tf(struct trapframe *tf, uintptr_t epc, uintptr_t cause, uintptr_t status) {
    uintptr_t *words = (uintptr_t *)tf;
    int i;

    for (i = 0; i < 36; i++) {
        words[i] = (uintptr_t)0xA5A5A5A5A5A50000ULL + (uintptr_t)(i * 0x101);
    }
    tf->epc = epc;
    tf->cause = cause;
    tf->status = status;
}

static void run_known(const char *tag, uintptr_t cause, volatile unsigned char *epc, unsigned delta,
                      uintptr_t status, const char *marker, const char *banned) {
    struct trapframe tf;
    struct trapframe before;
    char name[96];
    int how;

    if ((uintptr_t)epc == 0) {
        host_fail_hex(tag, 0);
        return;
    }
    fill_tf(&tf, (uintptr_t)epc, cause, status);
    before = tf;
    host_capture_reset();
    host_interrupt_reset();
    how = host_invoke(call_trap, &tf);
    if (how == HOST_FAULT) {
        join(name, tag, "读取");
        host_fail_text(name, host_fault_name());
        return;
    }
    join(name, tag, "panic");
    check_panic(how, 0, name);
    join(name, tag, "地址");
    check_marker_addr(host_capture(), marker, banned, before.epc, name);
    check_moved(tag, tf.epc, before.epc, delta);
    check_fields(tag, &before, &tf);
    join(name, tag, "中断");
    if (host_interrupt_count() != 0) {
        host_fail_hex(name, (unsigned long long)host_interrupt_count());
    } else {
        host_pass(name);
    }
}

static void run_unknown(const char *tag, uintptr_t cause, uintptr_t epc, uintptr_t status, int want_panic,
                        int zero_frame) {
    struct trapframe tf;
    struct trapframe before;
    char name[96];
    int how;

    if (zero_frame) {
        unsigned char *raw = (unsigned char *)&tf;
        size_t i;
        for (i = 0; i < sizeof(tf); i++) {
            raw[i] = 0;
        }
        tf.cause = cause;
    } else {
        fill_tf(&tf, epc, cause, status);
    }
    before = tf;
    host_capture_reset();
    host_interrupt_reset();
    how = host_invoke(call_trap, &tf);
    join(name, tag, "读取");
    if (how == HOST_FAULT) {
        host_fail_text(name, host_fault_name());
        return;
    }
    host_pass(name);
    join(name, tag, "输出");
    check_cause_line(host_capture(), before.cause, name);
    join(name, tag, "专属");
    check_exclusive(host_capture(), name);
    join(name, tag, "panic");
    check_panic(how, want_panic, name);
    check_moved(tag, tf.epc, before.epc, 0);
    check_fields(tag, &before, &tf);
    join(name, tag, "中断");
    if (host_interrupt_count() != 0) {
        host_fail_hex(name, (unsigned long long)host_interrupt_count());
    } else {
        host_pass(name);
    }
}

static unsigned long long load_le(volatile unsigned char *bytes, unsigned count) {
    unsigned long long value = 0;
    unsigned i;

    for (i = 0; i < count; i++) {
        value |= (unsigned long long)bytes[i] << (8 * i);
    }
    return value;
}

static void check_encoding(const char *name, volatile unsigned char *bytes, unsigned count,
                           unsigned long long encoding) {
    unsigned long long got = load_le(bytes, count);
    unsigned bits42 = (unsigned)((got >> 2) & 7u);

    if (got != encoding || (count == 4 && ((got & 0x7fu) == 0x7fu || (got & 3u) != 3u || bits42 == 7u)) ||
        (count == 2 && (got & 3u) == 3u)) {
        host_fail_hex(name, got);
        return;
    }
    host_pass(name);
}

static void run_both_modes(const char *tag, uintptr_t cause, volatile unsigned char *epc, unsigned delta,
                           const char *marker, const char *banned) {
    char name[96];

    join(name, tag, "用户");
    run_known(name, cause, epc, delta, SSTATUS_SPIE, marker, banned);
    join(name, tag, "内核");
    run_known(name, cause, epc, delta, SSTATUS_SPP | SSTATUS_SPIE, marker, banned);
}

static void run_cause_pair(const char *prefix, unsigned cause, uintptr_t epc) {
    char base[96];
    char tag[96];

    name_cause(base, prefix, cause);
    join(tag, base, "用户");
    run_unknown(tag, cause, epc, SSTATUS_SPIE, 0, 0);
    join(tag, base, "内核");
    run_unknown(tag, cause, epc, SSTATUS_SPP | SSTATUS_SPIE, 1, 0);
}

int main(void) {
    volatile unsigned char *page = host_page_write();
    void *guard = host_page_guard();
    volatile unsigned char *ebreak;
    volatile unsigned char *cebreak;
    volatile unsigned char *ill4;
    volatile unsigned char *ill2;
    volatile unsigned char *ill01;

    if (sizeof(struct trapframe) != 36 * sizeof(uintptr_t)) {
        host_fail_hex("trapframe字节", sizeof(struct trapframe));
        return 1;
    }
    host_pass("trapframe字节");

    if (page == 0 || guard == 0 || (uintptr_t)page == 0 || (uintptr_t)guard == 0 ||
        (uintptr_t)page == (uintptr_t)guard) {
        host_fail_hex("指令页", (unsigned long long)(uintptr_t)page);
        host_fail_hex("保护页", (unsigned long long)(uintptr_t)guard);
        return 1;
    }
    host_pass("指令页");

    ebreak = page;
    cebreak = page + 16;
    ill4 = page + 32;
    ill2 = page + 48;
    ill01 = page + 64;
    ebreak[0] = 0x73;
    ebreak[1] = 0x00;
    ebreak[2] = 0x10;
    ebreak[3] = 0x00;
    cebreak[0] = 0x02;
    cebreak[1] = 0x90;
    ill4[0] = 0x7b;
    ill4[1] = 0x00;
    ill4[2] = 0x00;
    ill4[3] = 0x00;
    ill2[0] = 0x00;
    ill2[1] = 0x00;
    ill01[0] = 0x01;
    ill01[1] = 0x61;

    check_encoding("ebreak编码", ebreak, 4, 0x00100073ULL);
    check_encoding("cebreak编码", cebreak, 2, 0x9002ULL);
    check_encoding("非法4编码", ill4, 4, 0x0000007bULL);
    check_encoding("非法2编码", ill2, 2, 0x0000ULL);
    check_encoding("非法01编码", ill01, 2, 0x6101ULL);

    run_both_modes("断点-ebreak", 3, ebreak, 4, "Breakpoint at", "Illegal instruction at");
    run_both_modes("断点-cebreak", 3, cebreak, 2, "Breakpoint at", "Illegal instruction at");
    run_both_modes("非法-4字节", 2, ill4, 4, "Illegal instruction at", "Breakpoint at");
    run_both_modes("非法-2字节", 2, ill2, 2, "Illegal instruction at", "Breakpoint at");
    run_both_modes("非法-01", 2, ill01, 2, "Illegal instruction at", "Breakpoint at");

    {
        static const unsigned rest[] = {0, 1, 4, 5, 6, 7, 8, 9, 11, 12, 13, 15};
        static const unsigned reserved[] = {10, 14, 16, 20, 31, 63};
        unsigned i;

        for (i = 0; i < sizeof(rest) / sizeof(rest[0]); i++) {
            run_cause_pair("其余-cause", rest[i], (uintptr_t)guard);
        }
        for (i = 0; i < sizeof(reserved) / sizeof(reserved[0]); i++) {
            run_cause_pair("保留-cause", reserved[i], (uintptr_t)guard);
        }
    }
    run_unknown("清零", 20, 0, 0, 0, 1);

    return host_failure_count() == 0 ? 0 : 1;
}
