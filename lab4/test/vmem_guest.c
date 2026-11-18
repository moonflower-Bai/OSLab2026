#include "vmem_fixture.h"

#include <riscv.h>
#include <sbi.h>
#include <string.h>

static const uintptr_t uart_base = 0x10000000;
static const uintptr_t test_va = 0x400ea000;
static const uintptr_t alias_va = 0x400eb000;
static const uint32_t writable = PTE_R | PTE_W | PTE_A | PTE_D;
static const uint32_t readonly = PTE_R | PTE_A;
static const char *active_case;

static volatile struct {
    bool armed;
    uintptr_t cause, va, pc, resume;
} expected_fault;

extern char vmem_load_pc[], vmem_load_resume[], vmem_store_pc[], vmem_store_resume[];
void vmem_probe_load(uintptr_t va, uint64_t *value);
void vmem_probe_store(uintptr_t va, uint64_t value);

static void uart_putchar(int character, void *opaque)
{
    volatile uint8_t *uart = (volatile uint8_t *)uart_base;
    (void)opaque;
    while (!(uart[5] & 0x20)) {}
    uart[0] = character;
}

int cprintf(const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    vprintfmt(uart_putchar, NULL, format, arguments);
    va_end(arguments);
    return 0;
}

static __noreturn void finish(bool passed)
{
    cprintf("VMEM_DONE %s\n", passed ? "PASS" : "FAIL");
    register uintptr_t reset_type asm("a0") = 0;
    register uintptr_t reset_reason asm("a1") = passed ? 0 : 1;
    register uintptr_t function asm("a6") = 0;
    register uintptr_t extension asm("a7") = 0x53525354;
    asm volatile("ecall" : "+r"(reset_type), "+r"(reset_reason)
                 : "r"(function), "r"(extension) : "memory");
    sbi_shutdown();
    for (;;) {
        asm volatile("wfi");
    }
}

static __noreturn void abort_case(void)
{
    if (active_case != NULL) {
        cprintf("VMEM_CASE %s FAIL\n", active_case);
    }
    finish(0);
}

void __panic(const char *file, int line, const char *format, ...)
{
    va_list arguments;
    cprintf("panic %s:%d: ", file, line);
    va_start(arguments, format);
    vprintfmt(uart_putchar, NULL, format, arguments);
    va_end(arguments);
    cprintf("\n");
    abort_case();
}

uintptr_t vmem_guest_trap(uintptr_t cause, uintptr_t va, uintptr_t pc)
{
    if (!expected_fault.armed || cause != expected_fault.cause || va != expected_fault.va ||
        pc != expected_fault.pc) {
        cprintf("unexpected trap: scause=%llu stval=0x%llx sepc=0x%llx; expected armed=%d scause=%llu stval=0x%llx sepc=0x%llx\n",
                cause, va, pc, expected_fault.armed, expected_fault.cause, expected_fault.va, expected_fault.pc);
        abort_case();
    }
    expected_fault.armed = 0;
    cprintf("VMEM_TRAP scause=%llu stval=0x%llx\n", cause, va);
    return expected_fault.resume;
}

static void arm_fault(uintptr_t cause, uintptr_t va, char *pc, char *resume)
{
    expected_fault.cause = cause;
    expected_fault.va = va;
    expected_fault.pc = (uintptr_t)pc;
    expected_fault.resume = (uintptr_t)resume;
    expected_fault.armed = 1;
}

static pde_t *start_sv39(void)
{
    vmem_reset();
    expected_fault.armed = 0;
    pde_t *root = vmem_root(0);
    root[PDX1(DRAM_BASE)] = pte_create(PPN(DRAM_BASE), writable | PTE_X);
    root[PDX1(uart_base)] = pte_create(0, writable);

    struct Page *middle_page = alloc_page();
    struct Page *leaf_page = alloc_page();
    assert(middle_page != NULL && leaf_page != NULL);
    set_page_ref(middle_page, 1);
    set_page_ref(leaf_page, 1);
    pte_t *middle = page2kva(middle_page);
    memset(middle, 0, PGSIZE);
    memset(page2kva(leaf_page), 0, PGSIZE);
    root[PDX1(test_va)] = ptd_create(page2ppn(middle_page));
    middle[PDX0(test_va)] = ptd_create(page2ppn(leaf_page));

    boot_pgdir_pa = (uintptr_t)root;
    uintptr_t satp_value = ((uintptr_t)SATP_MODE_SV39 << 60) | PPN(boot_pgdir_pa);
    write_csr(satp, satp_value);
    flush_tlb();
    assert(read_csr(satp) == satp_value);
    return root;
}

static bool map_guest(pde_t *root, struct Page *page, uintptr_t va, uint32_t perm)
{
    int result = page_insert(root, page, va, perm);
    VMEM_CHECK(result == 0, "page_insert at 0x%llx returned %d", va, result);
    return 1;
}

static bool replace_and_read(pde_t *root)
{
    const uint64_t marker_old = 0x132435465768798aULL;
    const uint64_t marker_new = 0xfedcba9876543210ULL;
    struct Page *old_page = vmem_claim_data(0);
    struct Page *new_page = vmem_claim_data(1);
    *(volatile uint64_t *)page2kva(old_page) = marker_old;
    *(volatile uint64_t *)page2kva(new_page) = marker_new;
    if (!map_guest(root, old_page, alias_va, writable) || !map_guest(root, old_page, test_va, writable)) {
        return 0;
    }
    uint64_t actual;
    vmem_probe_load(test_va, &actual);
    VMEM_CHECK(actual == marker_old, "initial read=0x%llx, expected A=0x%llx", actual, marker_old);
    if (!map_guest(root, new_page, test_va, writable)) {
        return 0;
    }
    vmem_probe_load(test_va, &actual);
    VMEM_CHECK(actual == marker_new, "read after replacement=0x%llx, expected B=0x%llx", actual, marker_new);
    vmem_probe_load(alias_va, &actual);
    VMEM_CHECK(actual == marker_old, "old alias read=0x%llx, expected A=0x%llx", actual, marker_old);
    return 1;
}

static bool protect_and_revoke(pde_t *root)
{
    const uint64_t marker = 0x52343c3a29206767ULL;
    struct Page *page = vmem_claim_data(0);
    if (!map_guest(root, page, test_va, writable)) {
        return 0;
    }
    uint64_t actual;
    vmem_probe_store(test_va, marker);
    vmem_probe_load(test_va, &actual);
    VMEM_CHECK(actual == marker, "writable mapping read=0x%llx, wrote 0x%llx", actual, marker);
    if (!map_guest(root, page, test_va, readonly)) {
        return 0;
    }
    vmem_probe_load(test_va, &actual);
    VMEM_CHECK(actual == marker, "read-only mapping read=0x%llx, expected 0x%llx", actual, marker);
    arm_fault(CAUSE_STORE_PAGE_FAULT, test_va, vmem_store_pc, vmem_store_resume);
    vmem_probe_store(test_va, ~marker);
    VMEM_CHECK(!expected_fault.armed, "store to read-only va=0x%llx returned without a fault", test_va);
    vmem_probe_load(test_va, &actual);
    VMEM_CHECK(actual == marker, "read after faulting store=0x%llx, expected 0x%llx", actual, marker);

    page_remove(root, test_va);
    arm_fault(CAUSE_LOAD_PAGE_FAULT, test_va, vmem_load_pc, vmem_load_resume);
    vmem_probe_load(test_va, &actual);
    VMEM_CHECK(!expected_fault.armed, "load after removal returned 0x%llx without a fault at va=0x%llx",
               actual, test_va);
    return 1;
}

void vmem_guest_main(void)
{
    static const struct {
        const char *name;
        bool (*run)(pde_t *root);
    } cases[] = {
        {"replace-and-read", replace_and_read},
        {"protect-and-revoke", protect_and_revoke},
    };
    bool all_passed = 1;
    cprintf("VMEM_BOOT\n");
    for (unsigned index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        active_case = cases[index].name;
        cprintf("VMEM_BEGIN %s\n", active_case);
        bool passed = cases[index].run(start_sv39());
        write_csr(satp, 0);
        flush_tlb();
        cprintf("VMEM_CASE %s %s\n", active_case, passed ? "PASS" : "FAIL");
        all_passed &= passed;
    }
    active_case = NULL;
    finish(all_passed);
}
