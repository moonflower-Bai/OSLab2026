#include <slub.h>
#include <pmm.h>
#include <list.h>
#include <stdio.h>

/* A teaching SLUB: per-size partial slabs and an in-object free chain.
 * No per-CPU state or locking: callers must serialize all operations.
 */
#define SLUB_MAGIC 0x534c55424c414232ULL
#define BITMAP_BYTES (PGSIZE / SLUB_ALIGNMENT / 8)

struct object_cache {
    size_t object_size;
    list_entry_t partial;
};

struct slab {
    uint64_t magic;
    struct Page *page;
    struct object_cache *cache;  /* NULL denotes a direct large allocation. */
    size_t npages;
    size_t inuse;
    size_t capacity;
    void *freelist;
    list_entry_t partial_link;
    list_entry_t all_link;
    unsigned char occupied[BITMAP_BYTES];
};

#define DATA_OFFSET ROUNDUP(sizeof(struct slab), SLUB_ALIGNMENT)

static struct object_cache caches[] = {
    {.object_size = 16},   {.object_size = 32},
    {.object_size = 64},   {.object_size = 128},
    {.object_size = 256},  {.object_size = 512},
    {.object_size = 1024}, {.object_size = 2048},
};
static list_entry_t all_slabs;
static bool initialized;

void
slub_init(void) {
    static_assert(SLUB_ALIGNMENT >= sizeof(void *));
    static_assert((SLUB_ALIGNMENT & (SLUB_ALIGNMENT - 1)) == 0);
    assert(!initialized);
    assert(DATA_OFFSET + SLUB_MAX_SMALL <= PGSIZE);
    list_init(&all_slabs);
    for (size_t i = 0; i < sizeof(caches) / sizeof(caches[0]); i++) {
        list_init(&caches[i].partial);
    }
    initialized = 1;
    cprintf("slub: 8 size caches, alignment %u, header %lu bytes\n",
            SLUB_ALIGNMENT, (unsigned long)DATA_OFFSET);
}

/* Metadata lives in pages obtained from the PMM, never from kmalloc itself. */
static struct slab *
new_span(size_t npages, struct object_cache *cache) {
    struct Page *page = alloc_pages(npages);
    if (page == NULL) {
        return NULL;
    }
    struct slab *slab = (struct slab *)(page2pa(page) + va_pa_offset);
    assert((uintptr_t)slab % PGSIZE == 0);
    slab->magic = SLUB_MAGIC;
    slab->page = page;
    slab->cache = cache;
    slab->npages = npages;
    slab->inuse = 0;
    slab->capacity = 0;
    slab->freelist = NULL;
    for (size_t i = 0; i < BITMAP_BYTES; i++) {
        slab->occupied[i] = 0;
    }
    list_init(&slab->partial_link);
    list_init(&slab->all_link);
    list_add(&all_slabs, &slab->all_link);
    return slab;
}

static struct slab *
new_small_slab(struct object_cache *cache) {
    struct slab *slab = new_span(1, cache);
    if (slab == NULL) {
        return NULL;
    }
    slab->capacity = (PGSIZE - DATA_OFFSET) / cache->object_size;
    assert(slab->capacity > 0 && slab->capacity <= BITMAP_BYTES * 8);
    for (size_t i = slab->capacity; i > 0; i--) {
        void *slot = (char *)slab + DATA_OFFSET + (i - 1) * cache->object_size;
        *(void **)slot = slab->freelist;
        slab->freelist = slot;
    }
    list_add(&cache->partial, &slab->partial_link);
    return slab;
}

static size_t
slot_index(struct slab *slab, void *ptr) {
    uintptr_t first = (uintptr_t)slab + DATA_OFFSET;
    assert((uintptr_t)ptr >= first);
    size_t offset = (uintptr_t)ptr - first;
    assert(offset % slab->cache->object_size == 0);
    size_t index = offset / slab->cache->object_size;
    assert(index < slab->capacity);
    return index;
}

void *
kmalloc(size_t size) {
    assert(initialized);
    if (size == 0) {
        return NULL;
    }
    if (size > SLUB_MAX_SMALL) {
        if (size > (size_t)-1 - DATA_OFFSET) {
            return NULL;
        }
        size_t total = size + DATA_OFFSET;
        size_t npages = total / PGSIZE + (total % PGSIZE != 0);
        struct slab *slab = new_span(npages, NULL);
        if (slab == NULL) {
            return NULL;
        }
        slab->inuse = 1;
        return (char *)slab + DATA_OFFSET;
    }

    size_t i = 0;
    while (caches[i].object_size < size) {
        i++;
    }
    struct object_cache *cache = &caches[i];
    struct slab *slab;
    if (list_empty(&cache->partial)) {
        slab = new_small_slab(cache);
        if (slab == NULL) {
            return NULL;
        }
    } else {
        slab = to_struct(list_next(&cache->partial), struct slab, partial_link);
    }
    assert(slab->inuse < slab->capacity && slab->freelist != NULL);
    void *slot = slab->freelist;
    size_t index = slot_index(slab, slot);
    unsigned int mask = 1U << (index % 8);
    assert(!(slab->occupied[index / 8] & mask));
    slab->freelist = *(void **)slot;
    slab->occupied[index / 8] |= mask;
    if (++slab->inuse == slab->capacity) {
        list_del_init(&slab->partial_link);
    }
    return slot;
}

/* Search our own registry before dereferencing metadata. This intentionally
 * favors simple ownership validation over an O(1) Page-to-slab lookup.
 */
static struct slab *
find_slab(void *ptr) {
    uintptr_t base = ROUNDDOWN((uintptr_t)ptr, PGSIZE);
    for (list_entry_t *le = list_next(&all_slabs); le != &all_slabs;
         le = list_next(le)) {
        struct slab *slab = to_struct(le, struct slab, all_link);
        if ((uintptr_t)slab == base) {
            return slab;
        }
    }
    return NULL;
}

static void
release_span(struct slab *slab) {
    struct Page *page = slab->page;
    size_t npages = slab->npages;
    list_del(&slab->all_link);
    slab->magic = 0;
    free_pages(page, npages);
    /* The slab header must not be accessed after returning these pages. */
}

void
kfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    assert(initialized);
    struct slab *slab = find_slab(ptr);
    assert(slab != NULL && slab->magic == SLUB_MAGIC);
    if (slab->cache == NULL) {
        assert(ptr == (char *)slab + DATA_OFFSET);
        release_span(slab);
        return;
    }

    size_t index = slot_index(slab, ptr);
    unsigned int mask = 1U << (index % 8);
    assert(slab->occupied[index / 8] & mask);
    assert(slab->inuse > 0);
    if (slab->inuse == slab->capacity) {
        list_add(&slab->cache->partial, &slab->partial_link);
    }
    slab->occupied[index / 8] &= ~mask;
    *(void **)ptr = slab->freelist;
    slab->freelist = ptr;
    if (--slab->inuse == 0) {
        list_del(&slab->partial_link);
        release_span(slab);
    }
}
