/* Host regression for the Lab2 buddy allocator.
 *
 * It compiles the real kern/mm/buddy_pmm.c against a small simulated page
 * array, then checks deterministic split/merge cases and a randomized
 * allocation/free model.  It is not part of the kernel image.
 */

#include <pmm.h>
#include <list.h>
#include <buddy_pmm.h>
#include <stdio.h>

extern int printf(const char *format, ...);
extern int vprintf(const char *format, va_list args);
extern void abort(void) __attribute__((noreturn));

#define TEST_PAGES 256
#define TEST_OPERATIONS 2000

static struct Page test_pages[TEST_PAGES];

struct Page *pages = test_pages;
size_t npage = TEST_PAGES;
const size_t nbase = 0;
uint64_t va_pa_offset = 0;
const struct pmm_manager *pmm_manager = &buddy_pmm_manager;

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
    printf("FAIL %s:%d: ", file, line);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    abort();
}

struct Page *
alloc_pages(size_t n) {
    return pmm_manager->alloc_pages(n);
}

void
free_pages(struct Page *base, size_t n) {
    pmm_manager->free_pages(base, n);
}

size_t
nr_free_pages(void) {
    return pmm_manager->nr_free_pages();
}

static unsigned int random_state = 0x4c414232;

static unsigned int
next_random(void) {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

static size_t
host_log2_ceil(size_t n) {
    size_t order = 0, size = 1;
    while (size < n) {
        size <<= 1;
        order++;
    }
    return order;
}

int
main(void) {
    pmm_manager->init();
    for (int i = 0; i < TEST_PAGES; i++) {
        test_pages[i].flags = 0;
        test_pages[i].property = 0;
        test_pages[i].ref = 0;
        list_init(&test_pages[i].page_link);
        SetPageReserved(&test_pages[i]);
    }
    pmm_manager->init_memmap(pages, TEST_PAGES);
    assert(nr_free_pages() == TEST_PAGES);
    pmm_manager->check();
    assert(nr_free_pages() == TEST_PAGES);

    /* Deterministic split / no-merge / merge cases. */
    struct Page *p0 = alloc_pages(1);
    struct Page *p1 = alloc_pages(1);
    assert(p0 == pages && p1 == pages + 1);
    assert(!PageProperty(p0) && !PageProperty(p1));
    assert(nr_free_pages() == TEST_PAGES - 2);

    free_pages(p0, 1);
    assert(PageProperty(p0));
    assert(!PageProperty(p1));
    assert(nr_free_pages() == TEST_PAGES - 1);

    free_pages(p1, 1);
    assert(nr_free_pages() == TEST_PAGES);

    /* Non-power-of-two request must reserve a whole 2^k block. */
    p0 = alloc_pages(5);
    assert(p0 == pages);
    assert(nr_free_pages() == TEST_PAGES - 8);
    free_pages(p0, 5);
    assert(nr_free_pages() == TEST_PAGES);

    p0 = alloc_pages(3);
    assert(p0 == pages);
    assert(nr_free_pages() == TEST_PAGES - 4);
    free_pages(p0, 3);
    assert(nr_free_pages() == TEST_PAGES);

    p0 = alloc_pages(TEST_PAGES);
    assert(p0 == pages);
    assert(nr_free_pages() == 0);
    assert(alloc_pages(1) == NULL);
    free_pages(p0, TEST_PAGES);
    assert(nr_free_pages() == TEST_PAGES);

    /* Randomized model: track the exact block size the buddy allocator
     * reserves, not just the requested page count. */
    struct allocation {
        int start;
        int size;
        int requested;
    } live[TEST_PAGES];
    int live_count = 0;
    int occupied[TEST_PAGES] = {0};
    int occupied_pages = 0;

    for (int step = 0; step < TEST_OPERATIONS; step++) {
        if (live_count > 0 && next_random() % 3 == 0) {
            int slot = (int)(next_random() % (unsigned int)live_count);
            struct allocation range = live[slot];
            free_pages(pages + range.start, (size_t)range.requested);
            for (int i = range.start; i < range.start + range.size; i++) {
                assert(occupied[i]);
                occupied[i] = 0;
            }
            occupied_pages -= range.size;
            live[slot] = live[--live_count];
        } else {
            size_t requested = next_random() % 16 + 1;
            struct Page *result = alloc_pages(requested);
            if (result != NULL) {
                int start = (int)(result - pages);
                int size = (int)((size_t)1 << host_log2_ceil(requested));
                assert(start >= 0 && start + size <= TEST_PAGES);
                for (int i = start; i < start + size; i++) {
                    assert(!occupied[i]);
                    occupied[i] = 1;
                }
                occupied_pages += size;
                live[live_count++] = (struct allocation){start, size, (int)requested};
            }
        }
        assert(nr_free_pages() == (size_t)(TEST_PAGES - occupied_pages));
    }

    while (live_count > 0) {
        struct allocation range = live[--live_count];
        free_pages(pages + range.start, (size_t)range.requested);
        for (int i = range.start; i < range.start + range.size; i++) {
            occupied[i] = 0;
        }
        occupied_pages -= range.size;
    }
    assert(nr_free_pages() == TEST_PAGES);
    pmm_manager->check();
    assert(nr_free_pages() == TEST_PAGES);

    printf("Buddy host test: PASS (%d operations, seed 0x4c414232)\n",
           TEST_OPERATIONS);
    return 0;
}
