/* Host adapter: the real SLUB and PMM operate on page-aligned simulated RAM.
 * No libc headers: ucore provides its own scalar and vararg definitions.
 */
#include <pmm.h>
#include <slub.h>
#include <default_pmm.h>
#include <stdio.h>

extern int printf(const char *format, ...);
extern int vprintf(const char *format, va_list args);
extern void exit(int status) __attribute__((noreturn));

#define TEST_PAGES 256

static struct Page test_pages[TEST_PAGES];
static unsigned char test_ram[TEST_PAGES * PGSIZE]
    __attribute__((aligned(PGSIZE)));

struct Page *pages = test_pages;
size_t npage = TEST_PAGES;
const size_t nbase = 0;
uint64_t va_pa_offset;
const struct pmm_manager *pmm_manager = &default_pmm_manager;

int
cprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

void __noreturn
__panic(const char *file, int line, const char *format, ...) {
    printf("HOST PANIC %s:%d: ", file, line);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    exit(86);  /* Runner distinguishes an expected panic from a sanitizer crash. */
}

struct Page *alloc_pages(size_t n) { return pmm_manager->alloc_pages(n); }
void free_pages(struct Page *base, size_t n) { pmm_manager->free_pages(base, n); }
size_t nr_free_pages(void) { return pmm_manager->nr_free_pages(); }

static bool
same_string(const char *left, const char *right) {
    while (*left != '\0' && *left == *right) {
        left++;
        right++;
    }
    return *left == *right;
}

static void
invalid_free(const char *mode) {
    if (same_string(mode, "foreign")) {
        int local = 0;
        kfree(&local);
    } else if (same_string(mode, "interior")) {
        char *ptr = kmalloc(32);
        assert(ptr != NULL);
        kfree(ptr + 1);
    } else if (same_string(mode, "large-interior")) {
        char *ptr = kmalloc(8193);
        assert(ptr != NULL);
        kfree(ptr + SLUB_ALIGNMENT);
    } else if (same_string(mode, "large-tail")) {
        char *ptr = kmalloc(8193);
        assert(ptr != NULL);
        kfree(ptr + PGSIZE);
    } else if (same_string(mode, "double-live-slab")) {
        void *ptr = kmalloc(32);
        assert(ptr != NULL && kmalloc(32) != NULL);
        kfree(ptr);
        kfree(ptr);
    } else if (same_string(mode, "double-empty-slab")) {
        void *ptr = kmalloc(32);
        assert(ptr != NULL);
        kfree(ptr);
        kfree(ptr);
    } else if (same_string(mode, "double-large")) {
        void *ptr = kmalloc(8193);
        assert(ptr != NULL);
        kfree(ptr);
        kfree(ptr);
    } else {
        printf("Unknown test mode: %s\n", mode);
        exit(2);
    }
    printf("FAIL: invalid free returned normally\n");
    exit(1);
}

int
main(int argc, char **argv) {
    va_pa_offset = (uintptr_t)test_ram;
    pmm_manager->init();
    for (size_t i = 0; i < TEST_PAGES; i++) {
        test_pages[i].flags = 0;
        SetPageReserved(&test_pages[i]);
        list_init(&test_pages[i].page_link);
    }
    /* Both registration and release must stop after appending a new tail.
     * The framework's original tail loops revisited their freshly added node.
     */
    pmm_manager->init_memmap(pages, TEST_PAGES / 2);
    pmm_manager->init_memmap(pages + TEST_PAGES / 2, TEST_PAGES / 2);
    struct Page *low = alloc_pages(TEST_PAGES / 2);
    struct Page *high = alloc_pages(TEST_PAGES / 2);
    assert(low == pages && high == pages + TEST_PAGES / 2);
    assert(nr_free_pages() == 0);
    free_pages(low, TEST_PAGES / 2);
    free_pages(high, TEST_PAGES / 2);
    struct Page *joined = alloc_pages(TEST_PAGES);
    assert(joined == pages);
    free_pages(joined, TEST_PAGES);
    printf("Host PMM: tail registration, tail release and coalescing PASS\n");
    pmm_manager->check();
    assert(nr_free_pages() == TEST_PAGES);
    slub_init();
    if (argc == 2) {
        invalid_free(argv[1]);
    }
    assert(argc == 1);
    slub_check();
    slub_stress(20000, 0x534c5542);
    assert(nr_free_pages() == TEST_PAGES);
    struct Page *all = alloc_pages(TEST_PAGES);
    assert(all == pages);  /* All pages and coalescing restored, not just a count. */
    free_pages(all, TEST_PAGES);
    printf("Host SLUB: PASS (256 pages restored, 20000 extra operations)\n");
    return 0;
}
