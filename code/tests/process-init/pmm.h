#ifndef TEST_PMM_H
#define TEST_PMM_H
#include <defs.h>
#include <mmu.h>
#include <memlayout.h>
#define CLONE_VM 0x100
extern uintptr_t boot_pgdir_pa;
extern char bootstack[];
struct Page *alloc_pages(size_t n);
void free_pages(struct Page *, size_t);
void *page2kva(struct Page *);
struct Page *kva2page(void *);
#endif
