#include "vmem_fixture.h"

#include <error.h>
#include <string.h>

static const uintptr_t active_va = 0x212000;
static const uintptr_t alias_va = 0x213000;
static const uint32_t user_rw = PTE_U | PTE_R | PTE_W;

static bool insert_at(pde_t *root, uintptr_t va, struct Page *page, uint32_t perm)
{
    int result = page_insert(root, page, va, perm);
    VMEM_CHECK(result == 0, "page_insert(0x%llx) returned %d, expected 0", va, result);
    return vmem_expect_leaf(root, va, page, perm);
}

static bool expect_allocator(unsigned requests, size_t free_pages_expected)
{
    VMEM_CHECK(vmem_alloc_requests() == requests && nr_free_pages() == free_pages_expected,
               "allocator: requests=%u free=%llu, expected requests=%u free=%llu",
               vmem_alloc_requests(), nr_free_pages(), requests, free_pages_expected);
    return 1;
}

static bool first_map(void)
{
    pde_t *root = vmem_root(0);
    struct Page *page = vmem_claim_data(0);
    size_t before = nr_free_pages();
    return insert_at(root, active_va, page, user_rw) &&
           vmem_expect_page(page, 1, 1, 0) && expect_allocator(2, before - 2);
}

static bool shared_alias(void)
{
    pde_t *root = vmem_root(0);
    struct Page *page = vmem_claim_data(0);
    if (!insert_at(root, active_va, page, user_rw) || !insert_at(root, alias_va, page, PTE_R) ||
        !vmem_expect_page(page, 2, 1, 0)) {
        return 0;
    }
    size_t before = nr_free_pages();
    unsigned requests = vmem_alloc_requests();
    page_remove(root, active_va);
    return vmem_expect_leaf(root, active_va, NULL, 0) &&
           vmem_expect_leaf(root, alias_va, page, PTE_R) &&
           vmem_expect_page(page, 1, 1, 0) && expect_allocator(requests, before);
}

static bool same_page_permission(void)
{
    pde_t *root = vmem_root(0);
    struct Page *page = vmem_claim_data(0);
    if (!insert_at(root, active_va, page, user_rw)) {
        return 0;
    }
    unsigned requests = vmem_alloc_requests();
    size_t before = nr_free_pages();
    return insert_at(root, active_va, page, PTE_R | PTE_X) &&
           vmem_expect_page(page, 1, 1, 0) && expect_allocator(requests, before);
}

static bool replace_page(void)
{
    pde_t *root = vmem_root(0);
    struct Page *old_page = vmem_claim_data(0);
    struct Page *new_page = vmem_claim_data(1);
    if (!insert_at(root, active_va, old_page, user_rw)) {
        return 0;
    }
    size_t before = nr_free_pages();
    unsigned requests = vmem_alloc_requests();
    return insert_at(root, active_va, new_page, PTE_R) &&
           vmem_expect_page(old_page, 0, 0, 1) && vmem_expect_page(new_page, 1, 1, 0) &&
           expect_allocator(requests, before + 1);
}

static bool remove_last_and_repeat(void)
{
    pde_t *root = vmem_root(0);
    struct Page *page = vmem_claim_data(0);
    if (!insert_at(root, active_va, page, user_rw)) {
        return 0;
    }
    size_t before = nr_free_pages();
    unsigned requests = vmem_alloc_requests();
    for (unsigned repetition = 0; repetition < 2; ++repetition) {
        page_remove(root, active_va);
        if (!vmem_expect_leaf(root, active_va, NULL, 0) || !vmem_expect_page(page, 0, 0, 1) ||
            !expect_allocator(requests, before + 1)) {
            cprintf("remove invocation=%u\n", repetition + 1);
            return 0;
        }
    }
    return 1;
}

static bool remove_holes(const uintptr_t *addresses, unsigned count, bool path_exists)
{
    pde_t *root = vmem_root(0);
    struct Page *page = vmem_claim_data(0);
    if (!insert_at(root, active_va, page, user_rw)) {
        return 0;
    }
    pte_t original = *vmem_walk(root, active_va);
    size_t before = nr_free_pages();
    unsigned requests = vmem_alloc_requests();
    for (unsigned index = 0; index < count; ++index) {
        uintptr_t va = addresses[index];
        pte_t *leaf = vmem_walk(root, va);
        VMEM_CHECK(path_exists ? leaf != NULL && !(*leaf & PTE_V) : leaf == NULL,
                   "invalid test setup at va=0x%llx: path=%d expected=%d",
                   va, leaf != NULL, path_exists);
        if (leaf != NULL) {
            *leaf = (page2ppn(page) << PTE_PPN_SHIFT) | PTE_R;
        }
        page_remove(root, va);
        if (!expect_allocator(requests, before) || !vmem_expect_leaf(root, va, NULL, 0) ||
            !vmem_expect_leaf(root, active_va, page, user_rw) || !vmem_expect_page(page, 1, 1, 0)) {
            cprintf("remove hole va=0x%llx\n", va);
            return 0;
        }
        pte_t guard_after = *vmem_walk(root, active_va);
        VMEM_CHECK(guard_after == original,
                   "remove hole va=0x%llx changed existing leaf to 0x%llx, expected 0x%llx",
                   va, guard_after, original);
    }
    return 1;
}

static bool remove_missing_path(void)
{
    const uintptr_t addresses[] = {0x40000000, 0x600000};
    return remove_holes(addresses, 2, 0);
}

static bool remove_invalid_leaf(void)
{
    return remove_holes(&alias_va, 1, 1);
}

static bool allocation_failure(unsigned nth)
{
    pde_t *root = vmem_root(0);
    pde_t *guard_root = vmem_root(1);
    struct Page *target = vmem_claim_data(0);
    struct Page *guard = vmem_claim_data(1);
    if (!insert_at(guard_root, active_va, guard, user_rw)) {
        return 0;
    }
    pte_t guard_before = *vmem_walk(guard_root, active_va);
    size_t before = nr_free_pages();
    unsigned requests = vmem_alloc_requests();
    vmem_fail_allocation(nth);
    int result = page_insert(root, target, active_va, PTE_R);
    vmem_fail_allocation(0);
    VMEM_CHECK(result == -E_NO_MEM, "allocation #%u: page_insert returned %d, expected %d",
               nth, result, -E_NO_MEM);
    if (!vmem_expect_leaf(root, active_va, NULL, 0) || !vmem_expect_page(target, 0, 1, 0) ||
        !vmem_expect_leaf(guard_root, active_va, guard, user_rw) || !vmem_expect_page(guard, 1, 1, 0)) {
        return 0;
    }
    pte_t guard_after = *vmem_walk(guard_root, active_va);
    VMEM_CHECK(guard_after == guard_before,
               "allocation #%u changed existing leaf to 0x%llx, expected 0x%llx", nth, guard_after, guard_before);

    size_t retained = 0;
    for (unsigned index = 0; index < NPDEENTRY; ++index) {
        pde_t entry = root[index];
        if (!(entry & PTE_V)) {
            continue;
        }
        VMEM_CHECK(nth == 2 && index == PDX1(active_va),
                   "allocation #%u: unexpected retained root[%u]=0x%llx", nth, index, entry);
        const pte_t *middle = vmem_table(entry);
        for (unsigned child = 0; child < NPDEENTRY; ++child) {
            VMEM_CHECK(!(middle[child] & PTE_V), "retained middle[%u] is not empty: 0x%llx",
                       child, middle[child]);
        }
        struct Page *table = pa2page(PTE_ADDR(entry));
        VMEM_CHECK(table->ref == 1, "retained middle table has ref=%d, expected 1", table->ref);
        ++retained;
    }
    if (!expect_allocator(requests + nth, before - retained) ||
        !insert_at(root, active_va, target, PTE_R) || !vmem_expect_page(target, 1, 1, 0)) {
        return 0;
    }

    page_remove(root, active_va);
    page_remove(guard_root, active_va);
    if (!vmem_expect_page(target, 0, 0, 1) || !vmem_expect_page(guard, 0, 0, 1)) {
        return 0;
    }
    size_t free_before_cleanup = nr_free_pages();
    VMEM_CHECK(free_before_cleanup == VMEM_POOL_PAGES - 4,
               "before table cleanup: free=%llu, expected %d", free_before_cleanup, VMEM_POOL_PAGES - 4);
    if (!vmem_release_tables(root) || !vmem_release_tables(guard_root)) {
        return 0;
    }
    VMEM_CHECK(nr_free_pages() == VMEM_POOL_PAGES,
               "after table cleanup: free=%llu, expected %d", nr_free_pages(), VMEM_POOL_PAGES);
    return 1;
}

static bool first_allocation_failure(void) { return allocation_failure(1); }
static bool second_allocation_failure(void) { return allocation_failure(2); }

int main(int argc, char **argv)
{
    static const struct {
        const char *name;
        bool (*run)(void);
    } cases[] = {
        {"first-map", first_map},
        {"shared-alias", shared_alias},
        {"same-page-permission", same_page_permission},
        {"replace-page", replace_page},
        {"remove-last-repeat", remove_last_and_repeat},
        {"remove-missing-path", remove_missing_path},
        {"remove-invalid-leaf", remove_invalid_leaf},
        {"alloc-fail-first", first_allocation_failure},
        {"alloc-fail-second", second_allocation_failure},
    };
    if (argc == 2) {
        for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            if (strcmp(argv[1], cases[index].name) == 0) {
                vmem_reset();
                return cases[index].run() ? 0 : 1;
            }
        }
    }
    cprintf("unknown or missing unit test name\n");
    return 2;
}
