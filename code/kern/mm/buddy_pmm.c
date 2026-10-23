#include <pmm.h>
#include <list.h>
#include <buddy_pmm.h>

/*
 * Buddy System physical page allocator.
 *
 * Free memory is split into power-of-two blocks.  Every order has its own
 * free list; only the first page of a block is linked into that list and
 * carries PG_property.  The block order is stored in page->property.
 *
 * An allocated block head is marked with a private flag bit and keeps its
 * order in page->property, so free_pages() can release the whole block even
 * when the caller only passes the originally requested page count.
 */

#define MAX_ORDER 15
#define PG_buddy_alloc 2
#define PG_buddy_check 3

#define SetPageBuddyAlloc(page) ((page)->flags |= (1UL << PG_buddy_alloc))
#define ClearPageBuddyAlloc(page) ((page)->flags &= ~(1UL << PG_buddy_alloc))
#define PageBuddyAlloc(page) (((page)->flags >> PG_buddy_alloc) & 1)

static free_area_t free_area[MAX_ORDER + 1];
static size_t nr_free;

static inline unsigned long
buddy_page_ppn(struct Page *page) {
    return (unsigned long)page2ppn(page);
}

static inline unsigned int
buddy_ctz(unsigned long n) {
    unsigned int bits = 0;
    assert(n != 0);
    while ((n & 1UL) == 0) {
        n >>= 1;
        bits++;
    }
    return bits;
}

static inline unsigned int
buddy_log2_floor(size_t n) {
    unsigned int order = 0;
    assert(n != 0);
    while (n > 1) {
        n >>= 1;
        order++;
    }
    return order;
}

static inline unsigned int
buddy_log2_ceil(size_t n) {
    unsigned int order = 0;
    size_t size = 1;
    assert(n != 0);
    while (size < n) {
        size <<= 1;
        order++;
    }
    return order;
}

static void
buddy_add_block(struct Page *base, unsigned int order) {
    base->property = order;
    SetPageProperty(base);
    list_add(&free_area[order].free_list, &base->page_link);
    free_area[order].nr_free += (unsigned int)(1UL << order);
    nr_free += (size_t)(1UL << order);
}

static void
buddy_remove_block(struct Page *base, unsigned int order) {
    assert(free_area[order].nr_free >= (unsigned int)(1UL << order));
    assert(nr_free >= (size_t)(1UL << order));
    list_del(&base->page_link);
    ClearPageProperty(base);
    base->property = 0;
    free_area[order].nr_free -= (unsigned int)(1UL << order);
    nr_free -= (size_t)(1UL << order);
}

static void
buddy_init(void) {
    for (unsigned int order = 0; order <= MAX_ORDER; order++) {
        list_init(&free_area[order].free_list);
        free_area[order].nr_free = 0;
    }
    nr_free = 0;
}

static void
buddy_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    for (struct Page *page = base; page != base + n; page++) {
        assert(PageReserved(page));
        page->flags = 0;
        page->property = 0;
        set_page_ref(page, 0);
        list_init(&page->page_link);
    }

    struct Page *page = base;
    size_t remaining = n;
    while (remaining > 0) {
        unsigned long ppn = buddy_page_ppn(page);
        unsigned int align_order = (ppn == 0) ? MAX_ORDER : buddy_ctz(ppn);
        unsigned int size_order = buddy_log2_floor(remaining);
        unsigned int order = align_order < size_order ? align_order : size_order;
        if (order > MAX_ORDER) {
            order = MAX_ORDER;
        }
        buddy_add_block(page, order);
        page += (size_t)1 << order;
        remaining -= (size_t)1 << order;
    }
}

static struct Page *
buddy_alloc_pages(size_t n) {
    if (n == 0 || n > nr_free) {
        return NULL;
    }

    unsigned int order = buddy_log2_ceil(n);
    if (order > MAX_ORDER) {
        return NULL;
    }

    unsigned int current = order;
    while (current <= MAX_ORDER && list_empty(&free_area[current].free_list)) {
        current++;
    }
    if (current > MAX_ORDER) {
        return NULL;
    }

    struct Page *page = le2page(list_next(&free_area[current].free_list), page_link);
    buddy_remove_block(page, current);

    while (current > order) {
        current--;
        struct Page *buddy = page + ((size_t)1 << current);
        buddy_add_block(buddy, current);
    }

    page->property = order;
    SetPageBuddyAlloc(page);
    return page;
}

static void
buddy_free_pages(struct Page *base, size_t n) {
    assert(n > 0);
    assert(PageBuddyAlloc(base));

    unsigned int order = base->property;
    assert(order <= MAX_ORDER);
    assert(n <= ((size_t)1 << order));

    size_t block_pages = (size_t)1 << order;
    for (struct Page *page = base; page != base + block_pages; page++) {
        assert(!PageReserved(page));
        assert(!PageProperty(page));
        page->flags = 0;
        page->property = 0;
        set_page_ref(page, 0);
    }

    struct Page *head = base;
    unsigned long ppn = buddy_page_ppn(base);
    while (order < MAX_ORDER) {
        unsigned long buddy_ppn = ppn ^ ((unsigned long)1 << order);
        if (buddy_ppn < nbase || buddy_ppn >= npage) {
            break;
        }
        struct Page *buddy = pages + (buddy_ppn - nbase);
        if (!PageProperty(buddy) || buddy->property != order) {
            break;
        }
        buddy_remove_block(buddy, order);
        if (buddy < head) {
            head = buddy;
            ppn = buddy_ppn;
        }
        order++;
    }

    buddy_add_block(head, order);
}

static size_t
buddy_nr_free_pages(void) {
    return nr_free;
}

static void
buddy_scan(void) {
    size_t total = 0;

    for (unsigned int order = 0; order <= MAX_ORDER; order++) {
        size_t order_total = 0;
        for (list_entry_t *le = list_next(&free_area[order].free_list);
             le != &free_area[order].free_list; le = list_next(le)) {
            struct Page *page = le2page(le, page_link);
            assert(PageProperty(page));
            assert(page->property == order);

            unsigned long ppn = buddy_page_ppn(page);
            assert(ppn >= nbase && ppn < npage);
            assert((ppn & ((1UL << order) - 1)) == 0);

            size_t block_pages = (size_t)1 << order;
            for (size_t i = 0; i < block_pages; i++) {
                struct Page *p = page + i;
                assert(!PageReserved(p));
                assert(!PageBuddyAlloc(p));
                if (i != 0) {
                    assert(!PageProperty(p));
                }
                assert(!(p->flags & (1UL << PG_buddy_check)));
                p->flags |= (1UL << PG_buddy_check);
                assert(page_ref(p) == 0);
            }

            if (order < MAX_ORDER) {
                unsigned long buddy_ppn = ppn ^ ((unsigned long)1 << order);
                if (buddy_ppn >= nbase && buddy_ppn < npage) {
                    struct Page *buddy = pages + (buddy_ppn - nbase);
                    assert(!(PageProperty(buddy) && buddy->property == order));
                }
            }

            order_total += block_pages;
        }
        assert((size_t)free_area[order].nr_free == order_total);
        total += order_total;
    }
    assert(total == nr_free);

    for (unsigned int order = 0; order <= MAX_ORDER; order++) {
        for (list_entry_t *le = list_next(&free_area[order].free_list);
             le != &free_area[order].free_list; le = list_next(le)) {
            struct Page *page = le2page(le, page_link);
            size_t block_pages = (size_t)1 << order;
            for (size_t i = 0; i < block_pages; i++) {
                (page + i)->flags &= ~(1UL << PG_buddy_check);
            }
        }
    }
}

static void
buddy_check(void) {
    buddy_scan();

    size_t before = nr_free;
    struct Page *p0 = buddy_alloc_pages(1);
    struct Page *p1 = buddy_alloc_pages(1);
    assert(p0 != NULL && p1 != NULL);
    assert(PageBuddyAlloc(p0) && PageBuddyAlloc(p1));
    buddy_free_pages(p0, 1);
    assert(PageBuddyAlloc(p1));
    buddy_free_pages(p1, 1);
    assert(nr_free == before);
    buddy_scan();

    struct Page *page = buddy_alloc_pages(5);
    assert(page != NULL);
    assert(PageBuddyAlloc(page));
    assert((buddy_page_ppn(page) & 7UL) == 0);
    buddy_free_pages(page, 5);
    assert(nr_free == before);

    page = buddy_alloc_pages(3);
    assert(page != NULL);
    assert(PageBuddyAlloc(page));
    assert((buddy_page_ppn(page) & 3UL) == 0);
    buddy_free_pages(page, 3);
    assert(nr_free == before);

    assert(buddy_alloc_pages(0) == NULL);
    assert(buddy_alloc_pages(nr_free + 1) == NULL);

    buddy_scan();
}

const struct pmm_manager buddy_pmm_manager = {
    .name = "buddy_pmm_manager",
    .init = buddy_init,
    .init_memmap = buddy_init_memmap,
    .alloc_pages = buddy_alloc_pages,
    .free_pages = buddy_free_pages,
    .nr_free_pages = buddy_nr_free_pages,
    .check = buddy_check,
};
