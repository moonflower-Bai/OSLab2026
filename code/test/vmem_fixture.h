#ifndef LAB4_VMEM_FIXTURE_H
#define LAB4_VMEM_FIXTURE_H

#include <pmm.h>
#include <stdio.h>

enum { VMEM_ROOTS = 2, VMEM_DATA_PAGES = 8, VMEM_POOL_PAGES = 64 };
#define VMEM_PERMS (PTE_R | PTE_W | PTE_X | PTE_U | PTE_G)

#define VMEM_CHECK(condition, ...)                    \
    do {                                              \
        if (!(condition)) {                           \
            cprintf(__VA_ARGS__);                      \
            cprintf("\n");                            \
            return 0;                                 \
        }                                             \
    } while (0)

void vmem_reset(void);
pde_t *vmem_root(unsigned index);
struct Page *vmem_data(unsigned slot);
struct Page *vmem_claim_data(unsigned slot);
bool vmem_owned(struct Page *page);
unsigned vmem_frees(struct Page *page);
unsigned vmem_alloc_requests(void);
void vmem_fail_allocation(unsigned nth);

pte_t *vmem_table(pde_t entry);
pte_t *vmem_walk(const pde_t *root, uintptr_t va);
bool vmem_release_tables(pde_t *root);
bool vmem_expect_leaf(const pde_t *root, uintptr_t va, struct Page *page, uint32_t perm);
bool vmem_expect_page(struct Page *page, int refs, bool owned, unsigned frees);

#ifndef VMEM_GUEST
extern void (*vmem_fence_hook)(bool all, uintptr_t va);
#endif

#endif
