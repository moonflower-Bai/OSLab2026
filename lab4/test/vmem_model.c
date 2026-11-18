#include "vmem_fixture.h"

#include <string.h>

enum { MODEL_VPNS = 8, MODEL_STEPS = 256, UNMAPPED = -1 };
enum opcode { MAP, REPLACE, PROTECT, REMOVE, QUERY };

struct mapping {
    int data_slot;
    uint32_t perm;
};

struct operation {
    enum opcode opcode;
    unsigned root;
    unsigned vpn;
    int data_slot;
    uint32_t perm;
};

static const uintptr_t addresses[MODEL_VPNS] = {
    0x001ff000, 0x00200000, 0x3ffff000, 0x40000000,
    0x001fe000, 0x00201000, 0x3fffe000, 0x40001000,
};
static const uint32_t permissions[] = {
    PTE_R, PTE_R | PTE_W, PTE_R | PTE_X, PTE_R | PTE_U, PTE_R | PTE_W | PTE_U,
};
static const struct operation prelude[] = {
    {MAP,     0, 0,  0, PTE_R | PTE_W},
    {MAP,     0, 1,  0, PTE_R | PTE_W},
    {MAP,     1, 0,  1, PTE_R},
    {MAP,     0, 2,  2, PTE_R},
    {MAP,     0, 3,  3, PTE_R | PTE_X},
    {REPLACE, 0, 0,  4, PTE_R | PTE_W},
    {PROTECT, 0, 0,  4, PTE_R},
    {REMOVE,  0, 0, UNMAPPED, 0},
    {QUERY,   1, 0, UNMAPPED, 0},
};

static struct mapping expected[VMEM_ROOTS][MODEL_VPNS];
static pte_t cached[VMEM_ROOTS][MODEL_VPNS];

static void invalidate(bool all, uintptr_t va)
{
    uintptr_t aligned_va = va & ~(uintptr_t)(PGSIZE - 1);
    for (unsigned root = 0; root < VMEM_ROOTS; ++root) {
        for (unsigned vpn = 0; vpn < MODEL_VPNS; ++vpn) {
            if (all || addresses[vpn] == aligned_va) {
                cached[root][vpn] = 0;
            }
        }
    }
}

static pte_t translate(unsigned root, unsigned vpn)
{
    pte_t *entry = &cached[root][vpn];
    if (!(*entry & PTE_V)) {
        const pte_t *leaf = vmem_walk(vmem_root(root), addresses[vpn]);
        *entry = leaf != NULL && (*leaf & PTE_V) ? *leaf : 0;
    }
    return *entry;
}

static void expected_references(int refs[VMEM_DATA_PAGES])
{
    memset(refs, 0, sizeof(int) * VMEM_DATA_PAGES);
    for (unsigned root = 0; root < VMEM_ROOTS; ++root) {
        for (unsigned vpn = 0; vpn < MODEL_VPNS; ++vpn) {
            int slot = expected[root][vpn].data_slot;
            if (slot != UNMAPPED) {
                ++refs[slot];
            }
        }
    }
}

static bool check_against_model(void)
{
    for (unsigned root = 0; root < VMEM_ROOTS; ++root) {
        for (unsigned vpn = 0; vpn < MODEL_VPNS; ++vpn) {
            struct mapping want = expected[root][vpn];
            struct Page *page = want.data_slot == UNMAPPED ? NULL : vmem_data(want.data_slot);
            if (!vmem_expect_leaf(vmem_root(root), addresses[vpn], page, want.perm)) {
                cprintf("page-table mismatch for root=%u\n", root);
                return 0;
            }
            pte_t effective = translate(root, vpn);
            uintptr_t want_pa = page == NULL ? 0 : page2pa(page);
            VMEM_CHECK(page == NULL ? !(effective & PTE_V) :
                           (effective & PTE_V) && PTE_ADDR(effective) == want_pa &&
                               (effective & VMEM_PERMS) == want.perm,
                       "stale TLB: root=%u va=0x%llx cached PTE=0x%llx; expected present=%d pa=0x%llx perm=0x%x",
                       root, addresses[vpn], effective, page != NULL, want_pa, want.perm);
        }
    }

    int refs[VMEM_DATA_PAGES];
    expected_references(refs);
    for (unsigned slot = 0; slot < VMEM_DATA_PAGES; ++slot) {
        struct Page *page = vmem_data(slot);
        VMEM_CHECK(page->ref == refs[slot] && vmem_owned(page) == (refs[slot] != 0),
                   "model refcount: data=%u observed ref=%d owned=%d; expected ref=%d owned=%d",
                   slot, page->ref, vmem_owned(page), refs[slot], refs[slot] != 0);
    }
    return 1;
}

static bool apply_operation(struct operation op)
{
    struct mapping *entry = &expected[op.root][op.vpn];
    pde_t *root = vmem_root(op.root);
    uintptr_t va = addresses[op.vpn];
    boot_pgdir_va = root;
    switch (op.opcode) {
    case MAP:
    case REPLACE:
    case PROTECT: {
        int refs[VMEM_DATA_PAGES];
        expected_references(refs);
        struct Page *page = refs[op.data_slot] == 0 ? vmem_claim_data(op.data_slot) : vmem_data(op.data_slot);
        int result = page_insert(root, page, va, op.perm);
        VMEM_CHECK(result == 0, "page_insert returned %d, expected 0", result);
        *entry = (struct mapping){op.data_slot, op.perm};
        break;
    }
    case REMOVE:
        page_remove(root, va);
        *entry = (struct mapping){UNMAPPED, 0};
        break;
    case QUERY:
        break;
    }
    return check_against_model();
}

static uint32_t next_random(uint32_t *state)
{
    uint32_t value = *state;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    return *state = value;
}

static struct operation random_operation(uint32_t *state)
{
    struct operation op = {.data_slot = UNMAPPED};
    op.opcode = next_random(state) % (QUERY + 1);
    op.root = next_random(state) % VMEM_ROOTS;
    op.vpn = next_random(state) % MODEL_VPNS;
    if (op.opcode == REMOVE || op.opcode == QUERY) {
        return op;
    }
    struct mapping current = expected[op.root][op.vpn];
    unsigned perm_count = sizeof(permissions) / sizeof(permissions[0]);
    unsigned perm_index = next_random(state) % perm_count;
    op.perm = permissions[perm_index];
    if (current.data_slot == UNMAPPED) {
        op.opcode = MAP;
        op.data_slot = next_random(state) % VMEM_DATA_PAGES;
    } else if (op.opcode == PROTECT) {
        op.data_slot = current.data_slot;
        if (op.perm == current.perm) {
            op.perm = permissions[(perm_index + 1) % perm_count];
        }
    } else {
        op.opcode = REPLACE;
        op.data_slot = (current.data_slot + 1 + next_random(state) % (VMEM_DATA_PAGES - 1)) % VMEM_DATA_PAGES;
    }
    return op;
}

int main(int argc, char **argv)
{
    char *end;
    const char *seed_arg = argc == 2 ? argv[1] : "";
    long value = strtol(seed_arg, &end, 16);
    if (value <= 0 || value > 0xffffffffUL || *end != '\0') {
        cprintf("expected a nonzero 32-bit hex seed\n");
        return 2;
    }
    uint32_t seed = value;
    uint32_t random_state = seed;
    vmem_reset();
    vmem_fence_hook = invalidate;
    for (unsigned root = 0; root < VMEM_ROOTS; ++root) {
        for (unsigned vpn = 0; vpn < MODEL_VPNS; ++vpn) {
            expected[root][vpn] = (struct mapping){UNMAPPED, 0};
        }
    }

    static const char *const op_names[] = {"map", "replace", "protect", "remove", "query"};
    for (unsigned step = 0; step < MODEL_STEPS; ++step) {
        struct operation op = step < sizeof(prelude) / sizeof(prelude[0])
                                  ? prelude[step] : random_operation(&random_state);
        cprintf("TRACE seed=0x%08x step=%u %s root=%u va=0x%llx data=%d perm=0x%x\n",
                seed, step + 1, op_names[op.opcode], op.root, addresses[op.vpn], op.data_slot, op.perm);
        if (!apply_operation(op)) {
            cprintf("model failed: seed=0x%08x step=%u\n", seed, step + 1);
            return 1;
        }
    }
    cprintf("VMEM_MODEL_DONE seed=0x%08x steps=%u\n", seed, (unsigned)MODEL_STEPS);
    return 0;
}
