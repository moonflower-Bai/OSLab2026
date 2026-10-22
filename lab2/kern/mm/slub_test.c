#include <slub.h>
#include <pmm.h>
#include <stdio.h>

/* Tests use public behavior and PMM counts, never the private slab layout. */
#define BATCH_OBJECTS 600
#define LIVE_OBJECTS 64
#define HELD_RANGES 64

static void *batch[BATCH_OBJECTS];

static unsigned char
pattern(size_t offset, unsigned int tag) {
    return (unsigned char)(tag + offset * 131 + (offset >> 8));
}

static void
write_pattern(void *ptr, size_t size, unsigned int tag) {
    unsigned char *bytes = ptr;
    for (size_t i = 0; i < size; i++) {
        bytes[i] = pattern(i, tag);
    }
}

static void
check_pattern(void *ptr, size_t size, unsigned int tag) {
    unsigned char *bytes = ptr;
    for (size_t i = 0; i < size; i++) {
        assert(bytes[i] == pattern(i, tag));
    }
}

static void
check_disjoint(void *a, size_t a_size, void *b, size_t b_size) {
    uintptr_t x = (uintptr_t)a, y = (uintptr_t)b;
    bool disjoint = x >= y ? x - y >= b_size : y - x >= a_size;
    if (!disjoint) {
        cprintf("overlapping objects: %lx size %lu, %lx size %lu\n",
                (unsigned long)x, (unsigned long)a_size,
                (unsigned long)y, (unsigned long)b_size);
    }
    assert(disjoint);
}

static void
check_boundaries(void) {
    static const size_t sizes[] = {
        1, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129,
        255, 256, 257, 511, 512, 513, 1023, 1024, 1025,
        2047, 2048, 2049, 4095, 4096, 4097, 8193,
    };
    size_t before = nr_free_pages();
    size_t count = sizeof(sizes) / sizeof(sizes[0]);
    for (size_t i = 0; i < count; i++) {
        batch[i] = kmalloc(sizes[i]);
        assert(batch[i] != NULL);
        assert((uintptr_t)batch[i] % SLUB_ALIGNMENT == 0);
        for (size_t j = 0; j < i; j++) {
            check_disjoint(batch[i], sizes[i], batch[j], sizes[j]);
        }
        write_pattern(batch[i], sizes[i], i + 1);
    }
    for (size_t i = count; i > 0; i--) {
        check_pattern(batch[i - 1], sizes[i - 1], i);
        kfree(batch[i - 1]);
    }
    assert(nr_free_pages() == before);
    cprintf("slub check: size boundaries, alignment and payload PASS\n");
}

static void
check_packing_and_reuse(void) {
    size_t before = nr_free_pages();
    size_t count = 0;
    /* Stop after the first object of a third slab: the first two are full.
     * Discover their capacity from page usage, not from a copied header size.
     */
    while (before - nr_free_pages() < 3) {
        assert(count < BATCH_OBJECTS);
        batch[count] = kmalloc(32);
        assert(batch[count] != NULL);
        write_pattern(batch[count], 32, count);
        count++;
    }
    assert(count > 3 && count < BATCH_OBJECTS);
    assert(before - nr_free_pages() == 3);
    kfree(batch[--count]);
    assert(nr_free_pages() == before - 2);

    void *victim = batch[0];
    kfree(victim);
    assert(nr_free_pages() == before - 2);
    batch[0] = kmalloc(32);
    assert(batch[0] == victim);  /* The only free slot in existing 32-byte slabs. */
    assert(nr_free_pages() == before - 2);
    write_pattern(batch[0], 32, 0);

    for (size_t parity = 0; parity < 2; parity++) {
        for (size_t i = parity; i < count; i += 2) {
            check_pattern(batch[i], 32, i);
            kfree(batch[i]);
        }
    }
    assert(nr_free_pages() == before);
    cprintf("slub check: packing, full/partial reuse and empty release PASS\n");
}

static unsigned int
next_random(unsigned int *state) {
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return *state;
}

void
slub_stress(size_t operations, unsigned int seed) {
    static const size_t sizes[] = {
        1, 7, 16, 17, 31, 63, 65, 127, 255, 257,
        511, 513, 1023, 1025, 2048, 2049, 4096, 4097, 8193,
    };
    static struct {
        void *ptr;
        size_t size;
        unsigned int tag;
    } live[LIVE_OBJECTS];
    size_t before = nr_free_pages();
    unsigned int state = seed;
    assert(seed != 0);
    for (size_t i = 0; i < LIVE_OBJECTS; i++) {
        live[i].ptr = NULL;
    }
    for (size_t step = 0; step < operations; step++) {
        size_t slot = next_random(&state) % LIVE_OBJECTS;
        if (live[slot].ptr != NULL) {
            check_pattern(live[slot].ptr, live[slot].size, live[slot].tag);
            kfree(live[slot].ptr);
            live[slot].ptr = NULL;
        } else {
            size_t size = sizes[next_random(&state) %
                                (sizeof(sizes) / sizeof(sizes[0]))];
            void *ptr = kmalloc(size);
            if (ptr != NULL) {
                assert((uintptr_t)ptr % SLUB_ALIGNMENT == 0);
                for (size_t i = 0; i < LIVE_OBJECTS; i++) {
                    if (live[i].ptr != NULL) {
                        check_disjoint(ptr, size, live[i].ptr, live[i].size);
                    }
                }
                live[slot].ptr = ptr;
                live[slot].size = size;
                live[slot].tag = next_random(&state);
                write_pattern(ptr, size, live[slot].tag);
            }
        }
        if (step % 64 == 0) {
            for (size_t i = 0; i < LIVE_OBJECTS; i++) {
                if (live[i].ptr != NULL) {
                    check_pattern(live[i].ptr, live[i].size, live[i].tag);
                }
            }
        }
    }
    for (size_t i = 0; i < LIVE_OBJECTS; i++) {
        if (live[i].ptr != NULL) {
            check_pattern(live[i].ptr, live[i].size, live[i].tag);
            kfree(live[i].ptr);
            live[i].ptr = NULL;
        }
    }
    assert(nr_free_pages() == before);
    cprintf("slub stress: PASS (%lu operations, seed 0x%x)\n",
            (unsigned long)operations, seed);
}

static void
check_exhaustion(void) {
    static struct Page *held[HELD_RANGES];
    static size_t lengths[HELD_RANGES];
    size_t before = nr_free_pages(), count = 0;
    void *guard = kmalloc(16);
    assert(guard != NULL);
    write_pattern(guard, 16, 0x5a);

    /* Take all remaining PMM pages in power-of-two runs. Fall back to smaller
     * runs if a particular size is unavailable. The lab starts with one range.
     */
    while (nr_free_pages() != 0) {
        assert(count < HELD_RANGES);
        size_t n = 1;
        while (n <= nr_free_pages() / 2) {
            n *= 2;
        }
        struct Page *page;
        while ((page = alloc_pages(n)) == NULL) {
            assert(n > 1);
            n /= 2;
        }
        held[count] = page;
        lengths[count++] = n;
    }
    void *cached = kmalloc(16);
    assert(cached != NULL && nr_free_pages() == 0);
    assert(kmalloc(512) == NULL);   /* No existing slab of this size. */
    assert(kmalloc(4097) == NULL);  /* Direct multi-page allocation. */
    check_pattern(guard, 16, 0x5a);
    kfree(cached);
    kfree(guard);
    while (count > 0) {
        count--;
        free_pages(held[count], lengths[count]);
    }
    assert(nr_free_pages() == before);
    void *again = kmalloc(4097);
    assert(again != NULL);
    write_pattern(again, 4097, 0xa5);
    check_pattern(again, 4097, 0xa5);
    kfree(again);
    assert(nr_free_pages() == before);
    cprintf("slub check: exhausted pages, cached slot and recovery PASS\n");
}

void
slub_check(void) {
    size_t before = nr_free_pages();
    assert(before >= 64);
    assert(kmalloc(0) == NULL);
    assert(kmalloc((size_t)-1) == NULL);
    assert(kmalloc((size_t)-1 - PGSIZE) == NULL);
    kfree(NULL);
    assert(nr_free_pages() == before);
    check_boundaries();
    check_packing_and_reuse();
    check_exhaustion();
    slub_stress(2000, 0x4c414232);
    assert(nr_free_pages() == before);
    cprintf("slub_check() succeeded! free pages restored: %lu\n",
            (unsigned long)before);
}
