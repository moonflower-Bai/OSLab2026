/* *
 * Lab2 P2 Best-Fit black-box tests (D1-D6).
 *
 * Only the interfaces listed in doc/lab2/p2_blackbox_api.md are used: the
 * allocator's free_list / nr_free and its static functions are never named.
 * The arena is isolated by draining every free page through alloc_pages(),
 * then carving [arena, arena + ARENA) out of one drained chunk with one held
 * page on each side.
 *
 * Every case starts and ends with the arena fully held and
 * nr_free_pages() == 0. A failed check prints the case, line and a dump of
 * the arena, then panics; a broken state is never "repaired".
 * */
#include <defs.h>
#include <stdio.h>
#include <string.h>
#include <pmm.h>
#include <best_fit_pmm.h>
#include <test_p2_blackbox.h>

#ifndef P2_CASE
#define P2_CASE 0
#endif

#define ARENA       64      // arena size; every case uses a prefix of it
#define OUTER_MAX   256     // max drained chunks remembered

static struct {
    struct Page *p;
    size_t n;
} outer[OUTER_MAX];
static int outer_cnt;
static size_t baseline;
static struct Page *arena;

static const char *cur_case;
static unsigned cur_span;
static int cases_run, cases_pass;

#define P(i)        (arena + (i))
#define OFF(pg)     ((unsigned)((pg) - arena))

/* ---------------------------------------------------------------------- */
/* failure reporting                                                       */
/* ---------------------------------------------------------------------- */

static void
dump_arena(void) {
    unsigned i;
    cprintf("  nr_free_pages() = %lu, span = %u\n",
            (unsigned long)nr_free_pages(), cur_span);
    for (i = 0; i < ARENA; i++) {
        struct Page *pg = P(i);
        if (PageProperty(pg) || PageReserved(pg) || page_ref(pg) != 0) {
            cprintf("  +%u: flags=0x%lx property=%u ref=%d\n", i,
                    (unsigned long)pg->flags, pg->property, page_ref(pg));
        }
    }
}

static void __noreturn
p2_fail(int line, const char *what) {
    cprintf("[P2] FAIL %s (test_p2_blackbox.c:%d): %s\n", cur_case, line, what);
    dump_arena();
    panic("P2 black-box case failed: %s", cur_case);
}

#define CHECK(x)                                    \
    do {                                            \
        if (!(x)) {                                 \
            p2_fail(__LINE__, #x);                  \
        }                                           \
    } while (0)

#define CHECK_AT(line, x, msg)                      \
    do {                                            \
        if (!(x)) {                                 \
            p2_fail((line), (msg));                 \
        }                                           \
    } while (0)

/* ---------------------------------------------------------------------- */
/* D1: invariant checker                                                   */
/*   B1 free-list ring is in ascending address order and matches the heads */
/*   B2 head PG_property=1, 0<property<=room; inner pages PG_property=0;   */
/*      every free page is not reserved and has ref 0                      */
/*   B3 sum(property) == nr_free_pages()                                   */
/*   B4 free blocks do not overlap                                         */
/*   B5 no two free blocks are address-adjacent                            */
/* ---------------------------------------------------------------------- */

struct blk {
    unsigned off, n;
};

static struct blk blk[ARENA];
static int nblk;
static list_entry_t *ring[ARENA + 2];

static bool
is_page_link(list_entry_t *le) {
    uintptr_t lo = (uintptr_t)pages;
    uintptr_t hi = (uintptr_t)(pages + (npage - nbase));
    uintptr_t a = (uintptr_t)le;
    if (a < lo || a >= hi) {
        return 0;
    }
    return (a - lo) % sizeof(struct Page) == offsetof(struct Page, page_link);
}

/* Returns NULL when B1-B5 hold, otherwise a description of the first
 * violation. Fills blk[0..nblk) with the free blocks in address order. */
static const char *
scan(void) {
    unsigned i, j, prev_end = 0;
    size_t sum = 0;
    int len = 0, sent = -1, k;
    list_entry_t *start, *le;

    nblk = 0;
    if (PageProperty(arena - 1) || PageProperty(arena + ARENA)) {
        return "guard page outside the arena became a free-block head";
    }
    for (i = 0; i < ARENA; i++) {
        struct Page *pg = P(i);
        if (!PageProperty(pg)) {
            continue;
        }
        if (nblk > 0 && i < prev_end) {
            return "B4: head lies inside the previous free block";
        }
        if (nblk > 0 && i == prev_end) {
            return "B5: two address-adjacent free blocks are not merged";
        }
        if (pg->property == 0 || pg->property > ARENA - i) {
            return "B2: head property is 0 or runs past the arena";
        }
        for (j = i; j < i + pg->property; j++) {
            if (PageReserved(P(j))) {
                return "B2: free page is still PageReserved";
            }
            if (page_ref(P(j)) != 0) {
                return "B2: free page has ref != 0";
            }
            if (j > i && PageProperty(P(j))) {
                return "B2/B4: PG_property set inside a free block";
            }
        }
        blk[nblk].off = i;
        blk[nblk].n = pg->property;
        nblk++;
        sum += pg->property;
        prev_end = i + pg->property;
    }
    if (sum != nr_free_pages()) {
        return "B3: sum of block property != nr_free_pages()";
    }
    if (nblk == 0) {
        return NULL;
    }

    /* walk the ring starting from the first head; exactly one node is the
     * allocator's sentinel, every other node must be one of our heads */
    start = &P(blk[0].off)->page_link;
    le = start;
    do {
        list_entry_t *nx;
        if (len >= nblk + 1) {
            return "B1: free-list ring has more nodes than heads + sentinel";
        }
        nx = le->next;
        if (nx == NULL || (uintptr_t)nx < KERNBASE) {
            return "list: next pointer is NULL or not a kernel address";
        }
        if (nx->prev != le) {
            return "list: next->prev does not point back";
        }
        if (is_page_link(le)) {
            struct Page *pg = le2page(le, page_link);
            if (pg < arena || pg >= arena + ARENA) {
                return "list: free list holds a page outside the arena";
            }
        } else {
            if (sent >= 0) {
                return "list: more than one non-page node on the ring";
            }
            sent = len;
        }
        ring[len++] = le;
        le = nx;
    } while (le != start);

    if (sent < 0) {
        return "list: ring has no sentinel";
    }
    if (len != nblk + 1) {
        return "B1: free-list nodes do not match PG_property heads";
    }
    for (k = 0; k < nblk; k++) {
        le = ring[(sent + 1 + k) % len];
        if (le2page(le, page_link) != P(blk[k].off)) {
            return "B1: free list is not in ascending address order";
        }
    }
    return NULL;
}

static void
scan_at(int line) {
    const char *e = scan();
    if (e != NULL) {
        p2_fail(line, e);
    }
}

#define SCAN() scan_at(__LINE__)

/* exact free-block layout: pairs of (offset, property) */
static void
expect_at(int line, int k, const unsigned *v) {
    int i;
    scan_at(line);
    CHECK_AT(line, nblk == k, "free-block count differs from expected");
    for (i = 0; i < k; i++) {
        CHECK_AT(line, blk[i].off == v[2 * i] && blk[i].n == v[2 * i + 1],
                 "free block (offset, property) differs from expected");
    }
}

#define EXPECT(...)                                                     \
    expect_at(__LINE__,                                                 \
              (int)(sizeof((unsigned[]){__VA_ARGS__}) / sizeof(unsigned) / 2), \
              (unsigned[]){__VA_ARGS__})
#define EXPECT_EMPTY() expect_at(__LINE__, 0, NULL)

/* ---------------------------------------------------------------------- */
/* operations with D4 page-state checks built in                           */
/* ---------------------------------------------------------------------- */

static void
free_at(int line, struct Page *base, size_t n) {
    size_t before = nr_free_pages(), i;
    CHECK_AT(line, base >= arena && base + n <= arena + cur_span,
             "test bug: free outside the case span");
    free_pages(base, n);
    CHECK_AT(line, nr_free_pages() == before + n,
             "nr_free_pages() did not grow by n after free_pages");
    for (i = 0; i < n; i++) {
        CHECK_AT(line, !PageReserved(base + i), "freed page is PageReserved");
        CHECK_AT(line, page_ref(base + i) == 0, "freed page has ref != 0");
    }
    scan_at(line);
}

static struct Page *
take_at(int line, size_t n) {
    size_t before = nr_free_pages(), i;
    struct Page *p = alloc_pages(n);
    if (p != NULL) {
        CHECK_AT(line, p >= arena && p + n <= arena + cur_span,
                 "alloc_pages returned pages outside the free arena span");
        CHECK_AT(line, nr_free_pages() == before - n,
                 "nr_free_pages() did not drop by n after alloc_pages");
        for (i = 0; i < n; i++) {
            CHECK_AT(line, !PageProperty(p + i),
                     "allocated page still has PG_property");
            CHECK_AT(line, !PageReserved(p + i),
                     "allocated page is PageReserved");
            CHECK_AT(line, page_ref(p + i) == 0,
                     "allocated page has ref != 0");
        }
    } else {
        CHECK_AT(line, nr_free_pages() == before,
                 "failed alloc_pages changed nr_free_pages()");
    }
    scan_at(line);
    return p;
}

static uint64_t snap_flags[ARENA];
static unsigned snap_prop[ARENA];
static int snap_ref[ARENA];

/* D6: a failing alloc must leave every arena page untouched */
static void
take_fail_at(int line, size_t n) {
    size_t before = nr_free_pages();
    unsigned i;
    struct Page *p;
    scan_at(line);
    for (i = 0; i < ARENA; i++) {
        snap_flags[i] = P(i)->flags;
        snap_prop[i] = P(i)->property;
        snap_ref[i] = page_ref(P(i));
    }
    p = alloc_pages(n);
    CHECK_AT(line, p == NULL, "alloc_pages should have returned NULL");
    CHECK_AT(line, nr_free_pages() == before,
             "failed alloc_pages changed nr_free_pages()");
    for (i = 0; i < ARENA; i++) {
        CHECK_AT(line, P(i)->flags == snap_flags[i],
                 "failed alloc_pages changed a page's flags");
        CHECK_AT(line, page_ref(P(i)) == snap_ref[i],
                 "failed alloc_pages changed a page's ref");
        if (PageProperty(P(i))) {
            CHECK_AT(line, P(i)->property == snap_prop[i],
                     "failed alloc_pages changed a head's property");
        }
    }
    scan_at(line);
}

/* take back every free page one at a time; all must come from the span */
static void
drain_at(int line, unsigned expected) {
    static bool seen[ARENA];
    unsigned got = 0;
    memset(seen, 0, sizeof(seen));
    while (nr_free_pages() > 0 && got <= cur_span) {
        struct Page *p = alloc_pages(1);
        CHECK_AT(line, p != NULL, "drain: alloc_pages(1) failed while pages are free");
        CHECK_AT(line, p >= arena && p < arena + cur_span,
                 "drain: page outside the case span");
        CHECK_AT(line, !seen[OFF(p)], "drain: same page returned twice");
        CHECK_AT(line, !PageProperty(p) && page_ref(p) == 0,
                 "drain: allocated page state is dirty");
        seen[OFF(p)] = 1;
        got++;
    }
    CHECK_AT(line, got == expected, "drain: unexpected number of free pages");
    CHECK_AT(line, nr_free_pages() == 0, "drain: nr_free_pages() != 0");
    CHECK_AT(line, alloc_pages(1) == NULL, "drain: alloc on empty set succeeded");
    expect_at(line, 0, NULL);
}

#define GIVE(off, n)        free_at(__LINE__, P(off), (n))
#define FREE(p, n)          free_at(__LINE__, (p), (n))
#define TAKE(n)             take_at(__LINE__, (n))
#define TAKE_FAIL(n)        take_fail_at(__LINE__, (n))
#define DRAIN(n)            drain_at(__LINE__, (n))

static void
begin(const char *name, unsigned span) {
    cur_case = name;
    cur_span = span;
    cases_run++;
    CHECK(span <= ARENA);
    CHECK(nr_free_pages() == 0);
    EXPECT_EMPTY();
}

static void
pass(void) {
    cases_pass++;
    cprintf("[P2] PASS %s\n", cur_case);
}

/* ---------------------------------------------------------------------- */
/* D0 / D1                                                                 */
/* ---------------------------------------------------------------------- */

static void
d0_manager(void) {
    begin("D0 manager is best_fit_pmm_manager", 1);
    CHECK(pmm_manager == &best_fit_pmm_manager);
    CHECK(strcmp(pmm_manager->name, "best_fit_pmm_manager") == 0);
    CHECK(pmm_manager->nr_free_pages() == nr_free_pages());
    DRAIN(0);
    pass();
}

/* mutate one page, the checker must object, then restore it exactly */
static void
expect_detect_at(int line, struct Page *pg, uint64_t flags, unsigned prop) {
    uint64_t old_flags = pg->flags;
    unsigned old_prop = pg->property;
    const char *e;
    pg->flags = flags;
    pg->property = prop;
    e = scan();
    pg->flags = old_flags;
    pg->property = old_prop;
    CHECK_AT(line, e != NULL, "checker self-test: mutation was not detected");
    cprintf("[P2]   checker caught +%u: %s\n", OFF(pg), e);
    scan_at(line);
}

#define DETECT(pg, flags, prop) expect_detect_at(__LINE__, (pg), (flags), (prop))
#define FPROP   (1UL << PG_property)
#define FRSV    (1UL << PG_reserved)

static void
d1_checker_selftest(void) {
    begin("D1 invariant checker detects corrupted states", 8);
    GIVE(0, 3);
    GIVE(5, 2);
    EXPECT(0, 3, 5, 2);
    DETECT(P(4), FPROP, 1);                 // fake head on a held page
    DETECT(P(1), FPROP, 1);                 // stale PG_property inside a block
    DETECT(P(0), FPROP, 4);                 // head property too large
    DETECT(P(5), 0, 2);                     // head lost PG_property
    DETECT(P(2), P(2)->flags | FRSV, P(2)->property); // free page reserved
    EXPECT(0, 3, 5, 2);
    DRAIN(5);
    pass();
}

/* ---------------------------------------------------------------------- */
/* D2: smallest-fit selection boundaries                                   */
/* ---------------------------------------------------------------------- */

static void
at_p2_01(void) {
    struct Page *p1, *p2;
    begin("D2 AT-P2-01 best fit is not first fit", 6);
    GIVE(0, 3);
    GIVE(4, 2);
    EXPECT(0, 3, 4, 2);
    CHECK(nr_free_pages() == 5);
    p1 = TAKE(2);
    CHECK(p1 == P(4));
    CHECK(nr_free_pages() == 3);
    EXPECT(0, 3);
    p2 = TAKE(2);
    CHECK(p2 == P(0));
    CHECK(nr_free_pages() == 1);
    EXPECT(2, 1);
    DRAIN(1);
    pass();
}

static void
at_p2_02(void) {
    struct Page *p1, *p2;
    begin("D2 AT-P2-02 tie between equal smallest blocks", 11);
    GIVE(0, 2);
    GIVE(4, 3);
    GIVE(9, 2);
    EXPECT(0, 2, 4, 3, 9, 2);
    CHECK(nr_free_pages() == 7);
    p1 = TAKE(2);
    p2 = TAKE(2);
    CHECK(p1 != p2);
    CHECK(p1 == P(0) || p1 == P(9));
    CHECK(p2 == P(0) || p2 == P(9));
    EXPECT(4, 3);
    CHECK(nr_free_pages() == 3);
    DRAIN(3);
    pass();
}

static void
d2_min_at_head(void) {
    struct Page *p;
    begin("D2 smallest fit at list head", 14);
    GIVE(0, 2);
    GIVE(3, 5);
    GIVE(9, 4);
    p = TAKE(2);
    CHECK(p == P(0));
    EXPECT(3, 5, 9, 4);
    DRAIN(9);
    pass();
}

static void
d2_min_at_tail(void) {
    struct Page *p;
    begin("D2 smallest fit at list tail", 14);
    GIVE(0, 5);
    GIVE(6, 4);
    GIVE(11, 2);
    p = TAKE(2);
    CHECK(p == P(11));
    EXPECT(0, 5, 6, 4);
    p = TAKE(3);                            // 4@6 beats the earlier 5@0
    CHECK(p == P(6));
    EXPECT(0, 5, 9, 1);
    DRAIN(6);
    pass();
}

static void
d2_skip_too_small(void) {
    struct Page *p, *q;
    begin("D2 skip too-small and larger blocks", 14);
    GIVE(0, 1);
    GIVE(2, 6);
    GIVE(9, 3);
    p = TAKE(2);                            // 1@0 too small, 6@2 not smallest
    CHECK(p == P(9));
    EXPECT(0, 1, 2, 6, 11, 1);
    q = TAKE(1);                            // tie between 1@0 and 1@11
    CHECK(q == P(0) || q == P(11));
    if (q == P(0)) {
        EXPECT(2, 6, 11, 1);
    } else {
        EXPECT(0, 1, 2, 6);
    }
    DRAIN(7);
    pass();
}

static void
d2_unique_exact(void) {
    struct Page *p;
    begin("D2 unique block exactly equals request", 4);
    GIVE(0, 4);
    p = TAKE(4);
    CHECK(p == P(0));
    EXPECT_EMPTY();
    TAKE_FAIL(1);
    FREE(p, 4);
    EXPECT(0, 4);
    DRAIN(4);
    pass();
}

static void
d2_tie_three(void) {
    struct Page *p[3];
    int i;
    begin("D2 three-way tie, larger block untouched", 15);
    GIVE(0, 2);
    GIVE(3, 2);
    GIVE(6, 2);
    GIVE(9, 4);
    for (i = 0; i < 3; i++) {
        p[i] = TAKE(2);
        CHECK(p[i] == P(0) || p[i] == P(3) || p[i] == P(6));
    }
    CHECK(p[0] != p[1] && p[0] != p[2] && p[1] != p[2]);
    EXPECT(9, 4);
    DRAIN(4);
    pass();
}

static void
d2_tie_split(void) {
    struct Page *p, *q;
    begin("D2 tie that needs a split", 8);
    GIVE(0, 3);
    GIVE(4, 3);
    p = TAKE(2);
    CHECK(p == P(0) || p == P(4));
    if (p == P(0)) {
        EXPECT(2, 1, 4, 3);
        q = TAKE(1);
        CHECK(q == P(2));
    } else {
        EXPECT(0, 3, 6, 1);
        q = TAKE(1);
        CHECK(q == P(6));
    }
    DRAIN(3);
    pass();
}

static void
at_p2_03(void) {
    struct Page *p1, *p2;
    begin("D2 AT-P2-03 split takes the front of the block", 5);
    GIVE(0, 5);
    p1 = TAKE(2);
    CHECK(p1 == P(0));
    EXPECT(2, 3);
    CHECK(PageProperty(P(2)));
    CHECK(!PageProperty(P(3)) && !PageProperty(P(4)));
    FREE(p1, 2);
    EXPECT(0, 5);
    p2 = TAKE(5);
    CHECK(p2 == P(0));
    DRAIN(0);
    pass();
}

static void
at_p2_04(void) {
    struct Page *p1;
    begin("D2 AT-P2-04 split remainder keeps its list position", 14);
    GIVE(0, 2);
    GIVE(4, 5);
    GIVE(11, 3);
    CHECK(nr_free_pages() == 10);
    p1 = TAKE(4);                           // only 5@4 fits
    CHECK(p1 == P(4));
    EXPECT(0, 2, 8, 1, 11, 3);
    GIVE(9, 2);                             // bridges 1@8 and 3@11
    EXPECT(0, 2, 8, 6);
    CHECK(nr_free_pages() == 8);
    DRAIN(8);
    pass();
}

static void
at_p2_13(void) {
    struct Page *p1, *p2;
    begin("D2 AT-P2-13 n=1, exact unique block, no zero-size rest", 3);
    GIVE(0, 3);
    p1 = TAKE(3);
    CHECK(p1 == P(0));
    EXPECT_EMPTY();
    CHECK(nr_free_pages() == 0);
    CHECK(!PageProperty(P(3)));             // page after the block untouched
    TAKE_FAIL(1);
    FREE(p1, 3);
    p2 = TAKE(1);
    CHECK(p2 == P(0));
    EXPECT(1, 2);
    FREE(p2, 1);
    EXPECT(0, 3);
    DRAIN(3);
    pass();
}

/* ---------------------------------------------------------------------- */
/* D3: merge paths                                                         */
/* ---------------------------------------------------------------------- */

static void
at_p2_05(void) {
    struct Page *p1;
    begin("D3 AT-P2-05 merge with successor only", 4);
    GIVE(2, 2);
    GIVE(0, 2);
    EXPECT(0, 4);
    CHECK(!PageProperty(P(2)));
    CHECK(nr_free_pages() == 4);
    p1 = TAKE(4);
    CHECK(p1 == P(0));
    FREE(p1, 4);
    DRAIN(4);
    pass();
}

static void
at_p2_06(void) {
    struct Page *p1;
    begin("D3 AT-P2-06 merge with predecessor only", 4);
    GIVE(0, 2);
    GIVE(2, 2);
    EXPECT(0, 4);
    CHECK(!PageProperty(P(2)));
    CHECK(nr_free_pages() == 4);
    p1 = TAKE(4);
    CHECK(p1 == P(0));
    FREE(p1, 4);
    DRAIN(4);
    pass();
}

static void
at_p2_07(void) {
    struct Page *p1;
    begin("D3 AT-P2-07 merge with both neighbours", 3);
    GIVE(0, 1);
    GIVE(2, 1);
    GIVE(1, 1);
    EXPECT(0, 3);
    CHECK(!PageProperty(P(1)) && !PageProperty(P(2)));
    CHECK(nr_free_pages() == 3);
    p1 = TAKE(3);
    CHECK(p1 == P(0));
    FREE(p1, 3);
    DRAIN(3);
    pass();
}

static void
at_p2_08(void) {
    struct Page *p1;
    begin("D3 AT-P2-08 no merge across a held page", 4);
    GIVE(0, 1);
    GIVE(2, 2);
    EXPECT(0, 1, 2, 2);
    CHECK(nr_free_pages() == 3);
    TAKE_FAIL(3);                           // enough in total, but split
    EXPECT(0, 1, 2, 2);
    p1 = TAKE(1);
    CHECK(p1 == P(0));
    EXPECT(2, 2);
    DRAIN(2);
    pass();
}

static void
d3_merge_chain(void) {
    struct Page *p;
    begin("D3 merge chain, then whole-block alloc", 9);
    GIVE(4, 1);                             // lone
    GIVE(6, 2);                             // lone, gap at 5
    GIVE(1, 2);                             // lone, gap at 3
    EXPECT(1, 2, 4, 1, 6, 2);
    GIVE(5, 1);                             // both sides: 4..7
    EXPECT(1, 2, 4, 4);
    GIVE(3, 1);                             // both sides: 1..7
    EXPECT(1, 7);
    GIVE(0, 1);                             // successor only
    EXPECT(0, 8);
    GIVE(8, 1);                             // predecessor only
    EXPECT(0, 9);
    p = TAKE(9);
    CHECK(p == P(0));
    EXPECT_EMPTY();
    FREE(p, 9);
    EXPECT(0, 9);
    DRAIN(9);
    pass();
}

/* ---------------------------------------------------------------------- */
/* D4: flag / property pollution                                           */
/* ---------------------------------------------------------------------- */

static void
at_p2_09(void) {
    struct Page *p1;
    int i;
    begin("D4 AT-P2-09 absorbed head flag is cleared", 4);
    GIVE(0, 2);
    GIVE(2, 2);
    CHECK(!PageProperty(P(2)));
    p1 = TAKE(4);
    CHECK(p1 == P(0));
    for (i = 0; i < 4; i++) {
        CHECK(!PageProperty(p1 + i));
    }
    FREE(p1, 4);
    EXPECT(0, 4);
    DRAIN(4);
    pass();
}

static void
d4_split_merge_flags(void) {
    struct Page *a, *b, *c, *all;
    begin("D4 flags across split and out-of-order merge", 12);
    GIVE(0, 12);
    a = TAKE(3);
    CHECK(a == P(0));
    EXPECT(3, 9);
    b = TAKE(4);
    CHECK(b == P(3));
    EXPECT(7, 5);
    c = TAKE(5);
    CHECK(c == P(7));
    EXPECT_EMPTY();
    FREE(b, 4);
    EXPECT(3, 4);
    FREE(a, 3);
    EXPECT(0, 7);
    CHECK(!PageProperty(P(3)));
    FREE(c, 5);
    EXPECT(0, 12);
    CHECK(!PageProperty(P(7)));
    all = TAKE(12);
    CHECK(all == P(0));
    FREE(all, 12);
    DRAIN(12);
    pass();
}

static void
d4_ref_cleared(void) {
    struct Page *p;
    begin("D4 free_pages clears ref", 4);
    GIVE(0, 4);
    p = TAKE(2);
    CHECK(p == P(0));
    set_page_ref(p, 1);
    set_page_ref(p + 1, 3);
    FREE(p, 2);                             // free_at checks ref == 0
    EXPECT(0, 4);
    DRAIN(4);
    pass();
}

static void
at_p2_10(void) {
    struct Page *p1;
    begin("D4 AT-P2-10 insertion order is address order", 5);
    GIVE(0, 1);
    GIVE(2, 1);
    GIVE(4, 1);
    EXPECT(0, 1, 2, 1, 4, 1);
    DRAIN(3);
    GIVE(4, 1);
    GIVE(2, 1);
    GIVE(0, 1);
    EXPECT(0, 1, 2, 1, 4, 1);
    GIVE(1, 1);
    EXPECT(0, 3, 4, 1);
    GIVE(3, 1);
    EXPECT(0, 5);
    p1 = TAKE(5);
    CHECK(p1 == P(0));
    DRAIN(0);
    pass();
}

static void
at_p2_14(void) {
    struct Page *b;
    unsigned i;
    begin("D4 AT-P2-14 init_memmap registered out of order", 16);
    for (i = 0; i < 12; i++) {
        if (i < 6 || i >= 8) {
            SetPageReserved(P(i));
        }
    }
    best_fit_pmm_manager.init_memmap(P(8), 4);
    CHECK(nr_free_pages() == 4);
    best_fit_pmm_manager.init_memmap(P(0), 6);
    CHECK(nr_free_pages() == 10);
    EXPECT(0, 6, 8, 4);
    for (i = 0; i < 12; i++) {
        if (i < 6 || i >= 8) {
            CHECK(!PageReserved(P(i)) && page_ref(P(i)) == 0);
            if (i != 0 && i != 8) {
                CHECK(P(i)->property == 0);
            }
        }
    }
    TAKE_FAIL(7);
    b = TAKE(6);
    CHECK(b == P(0));
    EXPECT(8, 4);
    DRAIN(4);
    pass();
}

/* ---------------------------------------------------------------------- */
/* D5: randomized model check                                              */
/* ---------------------------------------------------------------------- */

#define R_SPAN      48
#define R_OPS       1500

enum { OWN_FREE = 0, OWN_OUTER, OWN_ALLOC };

static unsigned char own[ARENA];
static struct blk rec[ARENA];
static int nrec;
static struct blk mrun[ARENA];
static int nmrun;
static uint64_t rng;

static unsigned
rnd(unsigned bound) {
    rng = rng * 6364136223846793005ULL + 1442695040888963407ULL;
    return (unsigned)(rng >> 33) % bound;
}

/* expected free blocks = maximal runs of free pages in the model */
static unsigned
model_runs(void) {
    unsigned i = 0, total = 0;
    nmrun = 0;
    while (i < R_SPAN) {
        if (own[i] != OWN_FREE) {
            i++;
            continue;
        }
        mrun[nmrun].off = i;
        while (i < R_SPAN && own[i] == OWN_FREE) {
            i++;
        }
        mrun[nmrun].n = i - mrun[nmrun].off;
        total += mrun[nmrun].n;
        nmrun++;
    }
    return total;
}

static void
model_compare(int line) {
    unsigned total = model_runs();
    int i;
    scan_at(line);
    CHECK_AT(line, nr_free_pages() == total, "D5: nr_free_pages() != model");
    CHECK_AT(line, nblk == nmrun, "D5: free-block count != model");
    for (i = 0; i < nblk; i++) {
        CHECK_AT(line, blk[i].off == mrun[i].off && blk[i].n == mrun[i].n,
                 "D5: free-block layout != model (missed or wrong merge)");
    }
}

static void
d5_random_round(uint64_t seed) {
    unsigned i, op, total;
    begin("D5 random model check", R_SPAN);
    rng = seed;
    nrec = 0;
    for (i = 0; i < R_SPAN; i++) {
        own[i] = OWN_OUTER;
    }
    /* start fragmented: free a random pattern of the held span */
    for (i = 0; i < R_SPAN;) {
        unsigned len = 1 + rnd(4);
        if (len > R_SPAN - i) {
            len = R_SPAN - i;
        }
        if (rnd(2)) {
            free_at(__LINE__, P(i), len);
            memset(own + i, OWN_FREE, len);
            model_compare(__LINE__);
        }
        i += len;
    }
    for (op = 0; op < R_OPS; op++) {
        unsigned r = rnd(100);
        if (r < 45) {
            unsigned n = rnd(10) == 0 ? 1 + rnd(R_SPAN) : 1 + rnd(6);
            unsigned best = 0;
            int k;
            struct Page *p;
            model_runs();
            for (k = 0; k < nmrun; k++) {
                if (mrun[k].n >= n && (best == 0 || mrun[k].n < best)) {
                    best = mrun[k].n;
                }
            }
            if (best == 0) {
                TAKE_FAIL(n);
            } else {
                bool ok = 0;
                p = TAKE(n);
                CHECK(p != NULL);
                for (k = 0; k < nmrun; k++) {
                    if (mrun[k].off == OFF(p) && mrun[k].n == best) {
                        ok = 1;
                    }
                }
                if (!ok) {
                    cprintf("[P2]   seed=0x%lx op=%u n=%u got +%u, smallest fit=%u\n",
                            (unsigned long)seed, op, n, OFF(p), best);
                }
                CHECK(ok);  // must start a smallest fitting block
                memset(own + OFF(p), OWN_ALLOC, n);
                rec[nrec].off = OFF(p);
                rec[nrec].n = n;
                nrec++;
            }
        } else if (r < 80) {
            if (nrec > 0) {
                int k = (int)rnd(nrec);
                FREE(P(rec[k].off), rec[k].n);
                memset(own + rec[k].off, OWN_FREE, rec[k].n);
                rec[k] = rec[--nrec];
            }
        } else {
            unsigned s = rnd(R_SPAN), len = 0, want = 1 + rnd(4);
            while (s < R_SPAN && own[s] != OWN_OUTER) {
                s++;
            }
            while (s + len < R_SPAN && own[s + len] == OWN_OUTER && len < want) {
                len++;
            }
            if (len > 0) {
                GIVE(s, len);
                memset(own + s, OWN_FREE, len);
            }
        }
        model_compare(__LINE__);
    }
    total = model_runs();
    cprintf("[P2]   seed=0x%lx ops=%u free_at_end=%u held_allocs=%d\n",
            (unsigned long)seed, R_OPS, total, nrec);
    DRAIN(total);
    pass();
}

static void d5_seed1(void) { d5_random_round(0x2024b357ULL); }
static void d5_seed2(void) { d5_random_round(0x5eed0002ULL); }
static void d5_seed3(void) { d5_random_round(0xdeadbeefULL); }

/* ---------------------------------------------------------------------- */
/* D6: failure paths have no side effects                                  */
/* ---------------------------------------------------------------------- */

static void
d6_empty(void) {
    begin("D6 alloc on an empty free set", 4);
    TAKE_FAIL(1);
    TAKE_FAIL(4);
    TAKE_FAIL(1UL << 20);
    EXPECT_EMPTY();
    DRAIN(0);
    pass();
}

static void
at_p2_11(void) {
    struct Page *p1, *p2;
    begin("D6 AT-P2-11 nr_free accounting on split/fail", 5);
    GIVE(0, 5);
    p1 = TAKE(2);
    CHECK(p1 == P(0));
    CHECK(nr_free_pages() == 3);
    EXPECT(2, 3);
    p2 = TAKE(3);
    CHECK(p2 == P(2));
    CHECK(nr_free_pages() == 0);
    EXPECT_EMPTY();
    FREE(p1, 2);
    CHECK(nr_free_pages() == 2);
    EXPECT(0, 2);
    TAKE_FAIL(9);
    CHECK(nr_free_pages() == 2);
    EXPECT(0, 2);
    DRAIN(2);
    pass();
}

static void
at_p2_12(void) {
    struct Page *d;
    begin("D6 AT-P2-12 failed allocs leave state intact", 7);
    GIVE(0, 2);
    GIVE(4, 3);
    TAKE_FAIL(6);                           // more than total
    TAKE_FAIL(5);                           // equal to total, scattered
    TAKE_FAIL(4);                           // less than total, no block fits
    EXPECT(0, 2, 4, 3);
    CHECK(nr_free_pages() == 5);
    d = TAKE(3);
    CHECK(d == P(4));
    DRAIN(2);
    pass();
}

static void
d6_over_total(void) {
    struct Page *p;
    begin("D6 n > nr_free and huge n", 8);
    GIVE(0, 3);
    GIVE(4, 4);
    TAKE_FAIL(8);
    TAKE_FAIL(1UL << 20);
    TAKE_FAIL(7);
    TAKE_FAIL(5);
    EXPECT(0, 3, 4, 4);
    p = TAKE(4);
    CHECK(p == P(4));
    EXPECT(0, 3);
    DRAIN(3);
    pass();
}

/* ---------------------------------------------------------------------- */
/* isolation and driver                                                    */
/* ---------------------------------------------------------------------- */

static void
isolate(void) {
    size_t left;
    int i;
    cur_case = "isolation";
    baseline = nr_free_pages();
    CHECK(baseline > 0);
    outer_cnt = 0;
    while ((left = nr_free_pages()) > 0) {
        struct Page *p = NULL;
        size_t n;
        for (n = left; n >= 1; n--) {
            if ((p = alloc_pages(n)) != NULL) {
                break;
            }
        }
        CHECK(p != NULL);
        CHECK(outer_cnt < OUTER_MAX);
        outer[outer_cnt].p = p;
        outer[outer_cnt].n = n;
        outer_cnt++;
    }
    CHECK(alloc_pages(1) == NULL);
    arena = NULL;
    for (i = 0; i < outer_cnt; i++) {
        if (outer[i].n >= ARENA + 2) {
            arena = outer[i].p + 1;
            break;
        }
    }
    CHECK(arena != NULL);
    cprintf("[P2] isolated: baseline=%lu chunks=%d arena=+%lu..+%lu\n",
            (unsigned long)baseline, outer_cnt,
            (unsigned long)(arena - pages), (unsigned long)(arena - pages + ARENA));
}

static void
release(void) {
    int i;
    cur_case = "release";
    CHECK(nr_free_pages() == 0);
    for (i = 0; i < outer_cnt; i++) {
        free_pages(outer[i].p, outer[i].n);
    }
    CHECK(nr_free_pages() == baseline);
}

static void (*const cases[])(void) = {
    d0_manager,
    d1_checker_selftest,
    at_p2_01,
    at_p2_02,
    d2_min_at_head,
    d2_min_at_tail,
    d2_skip_too_small,
    d2_unique_exact,
    d2_tie_three,
    d2_tie_split,
    at_p2_03,
    at_p2_04,
    at_p2_13,
    at_p2_05,
    at_p2_06,
    at_p2_07,
    at_p2_08,
    d3_merge_chain,
    at_p2_09,
    d4_split_merge_flags,
    d4_ref_cleared,
    at_p2_10,
    at_p2_14,
    d5_seed1,
    d5_seed2,
    d5_seed3,
    d6_empty,
    at_p2_11,
    at_p2_12,
    d6_over_total,
};

#define NCASES ((int)(sizeof(cases) / sizeof(cases[0])))

void
test_p2_blackbox(void) {
    int i;
    cprintf("[P2] black-box suite start (P2_CASE=%d, %d cases)\n", P2_CASE, NCASES);
    if (P2_CASE < 0 || P2_CASE > NCASES) {
        cprintf("[P2] no such case\n");
        cprintf("P2-BLACKBOX-DONE\n");
        return;
    }
    isolate();
    for (i = 0; i < NCASES; i++) {
        if (P2_CASE == 0 || P2_CASE == i + 1) {
            cases[i]();
        }
    }
    release();
    cprintf("[P2] SUMMARY run=%d pass=%d fail=%d\n", cases_run, cases_pass,
            cases_run - cases_pass);
    cprintf("P2-BLACKBOX-DONE\n");
}
