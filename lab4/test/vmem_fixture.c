#include "vmem_fixture.h"

#include <string.h>

#ifdef VMEM_GUEST
enum { MAX_RESERVED_PAGES = 1024 };
#else
enum { MAX_RESERVED_PAGES = 0 };
#endif

_Static_assert(sizeof(pte_t) * NPTEENTRY == PGSIZE, "VMEM tests require 64-bit PTEs");

static struct Page descriptors[MAX_RESERVED_PAGES + VMEM_POOL_PAGES];
static struct Page *pool;
static _Alignas(PGSIZE) pte_t frames[VMEM_POOL_PAGES][NPTEENTRY];
static _Alignas(PGSIZE) pde_t roots[VMEM_ROOTS][NPDEENTRY];
static bool owned[VMEM_POOL_PAGES];
static unsigned frees[VMEM_POOL_PAGES];
static unsigned requests, fail_request;

static size_t pool_index(struct Page *page)
{
    uintptr_t offset = (uintptr_t)page - (uintptr_t)pool;
    if (offset >= sizeof(*pool) * VMEM_POOL_PAGES || offset % sizeof(*pool) != 0) {
        panic("page descriptor %p is outside the test pool", (void *)page);
    }
    return offset / sizeof(*pool);
}

static struct Page *test_alloc_pages(size_t count)
{
    if (count != 1) {
        panic("test allocator expected one page, got %llu", count);
    }
    if (++requests == fail_request) {
        return NULL;
    }
    for (size_t index = VMEM_DATA_PAGES; index < VMEM_POOL_PAGES; ++index) {
        if (!owned[index]) {
            owned[index] = 1;
            memset(frames[index], 0x5a, PGSIZE);
            return &pool[index];
        }
    }
    return NULL;
}

static void test_free_pages(struct Page *page, size_t count)
{
    size_t index = pool_index(page);
    if (count != 1 || !owned[index]) {
        panic("invalid free: page=%llu count=%llu owned=%d ref=%d",
              index, count, owned[index], page->ref);
    }
    page->ref = 0;
    owned[index] = 0;
    ++frees[index];
}

static size_t test_nr_free_pages(void)
{
    size_t total = 0;
    for (size_t index = 0; index < VMEM_POOL_PAGES; ++index) {
        total += !owned[index];
    }
    return total;
}

static const struct pmm_manager test_pmm = {
    .name = "vmem-test-pool",
    .alloc_pages = test_alloc_pages,
    .free_pages = test_free_pages,
    .nr_free_pages = test_nr_free_pages,
};

void vmem_reset(void)
{
    size_t reserved = 0;
#ifdef VMEM_GUEST
    reserved = PPN((uintptr_t)frames) - nbase;
    if (reserved > MAX_RESERVED_PAGES) {
        panic("guest frame pool starts outside the test RAM window: %p", (void *)frames);
    }
#endif
    pages = descriptors;
    pool = pages + reserved;
    npage = nbase + reserved + VMEM_POOL_PAGES;
    va_pa_offset = (uintptr_t)frames - page2pa(pool);
    memset(pool, 0, sizeof(*pool) * VMEM_POOL_PAGES);
    memset(roots, 0, sizeof(roots));
    memset(owned, 0, sizeof(owned));
    memset(frees, 0, sizeof(frees));
    requests = fail_request = 0;
    pmm_manager = &test_pmm;
    boot_pgdir_va = roots[0];
}

pde_t *vmem_root(unsigned index)
{
    return roots[index];
}

struct Page *vmem_data(unsigned slot)
{
    return &pool[slot];
}

struct Page *vmem_claim_data(unsigned slot)
{
    if (slot >= VMEM_DATA_PAGES || owned[slot] || pool[slot].ref != 0) {
        panic("cannot claim data page slot %u", slot);
    }
    owned[slot] = 1;
    return vmem_data(slot);
}

bool vmem_owned(struct Page *page)
{
    return owned[pool_index(page)];
}

unsigned vmem_frees(struct Page *page)
{
    return frees[pool_index(page)];
}

unsigned vmem_alloc_requests(void)
{
    return requests;
}

void vmem_fail_allocation(unsigned nth)
{
    fail_request = nth == 0 ? 0 : requests + nth;
}

pte_t *vmem_table(pde_t entry)
{
    uintptr_t offset = PTE_ADDR(entry) - page2pa(pool);
    size_t index = offset / PGSIZE;
    if ((entry & (PTE_V | PTE_R | PTE_W | PTE_X)) != PTE_V ||
        index < VMEM_DATA_PAGES || index >= VMEM_POOL_PAGES || !owned[index]) {
        panic("invalid intermediate PTE: 0x%llx", entry);
    }
    return frames[index];
}

pte_t *vmem_walk(const pde_t *root, uintptr_t va)
{
    pte_t *table = NULL;
    const pte_t *entries = root;
    for (unsigned level = 2; level > 0; --level) {
        pte_t entry = entries[(va >> (PGSHIFT + 9 * level)) & 0x1ff];
        if (!(entry & PTE_V)) {
            return NULL;
        }
        entries = table = vmem_table(entry);
    }
    return &table[(va >> PGSHIFT) & 0x1ff];
}

static bool release_children(pte_t *table, unsigned level)
{
    for (unsigned index = 0; index < NPTEENTRY; ++index) {
        pte_t entry = table[index];
        if (!(entry & PTE_V)) {
            continue;
        }
        VMEM_CHECK(level > 0, "leaf %u still valid at cleanup: 0x%llx", index, entry);
        if (!release_children(vmem_table(entry), level - 1)) {
            return 0;
        }
        struct Page *page = pa2page(PTE_ADDR(entry));
        VMEM_CHECK(page->ref == 1, "table page PA=0x%llx at cleanup: ref=%d, expected 1",
                   page2pa(page), page->ref);
        table[index] = 0;
        page->ref = 0;
        free_page(page);
    }
    return 1;
}

bool vmem_release_tables(pde_t *root)
{
    return release_children(root, 2);
}

bool vmem_expect_leaf(const pde_t *root, uintptr_t va, struct Page *page, uint32_t perm)
{
    const pte_t *leaf = vmem_walk(root, va);
    pte_t actual = leaf == NULL ? 0 : *leaf;
    if (page == NULL) {
        VMEM_CHECK(!(actual & PTE_V), "va=0x%llx: unexpected valid PTE 0x%llx", va, actual);
    } else {
        VMEM_CHECK((actual & PTE_V) && PTE_ADDR(actual) == page2pa(page) &&
                       (actual & VMEM_PERMS) == perm,
                   "va=0x%llx: PTE=0x%llx pa=0x%llx perm=0x%llx; expected valid pa=0x%llx perm=0x%x",
                   va, actual, PTE_ADDR(actual), actual & VMEM_PERMS, page2pa(page), perm);
    }
    return 1;
}

bool vmem_expect_page(struct Page *page, int refs, bool live, unsigned free_events)
{
    size_t index = pool_index(page);
    VMEM_CHECK(page->ref == refs && owned[index] == live && frees[index] == free_events,
               "page=%llu: ref=%d owned=%d frees=%u; expected ref=%d owned=%d frees=%u",
               index, page->ref, owned[index], frees[index], refs, live, free_events);
    return 1;
}

#ifndef VMEM_GUEST
void (*vmem_fence_hook)(bool all, uintptr_t va);

void vmem_host_sfence_page(uintptr_t va)
{
    if (vmem_fence_hook != NULL) {
        vmem_fence_hook(0, va);
    }
}

void vmem_host_sfence_all(void)
{
    if (vmem_fence_hook != NULL) {
        vmem_fence_hook(1, 0);
    }
}
#endif
