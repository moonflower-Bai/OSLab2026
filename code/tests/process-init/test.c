#include <proc.h>
#include <pmm.h>
#include <error.h>
extern void *malloc(unsigned long);
extern void free(void *);
extern int printf(const char *, ...);
extern void abort(void) __attribute__((noreturn));
static int fail_pcb, fail_stack, pcb_live, stack_live;
struct mock_page { struct Page page; char stack[KSTACKSIZE] __attribute__((aligned(16))); };
uintptr_t boot_pgdir_pa = 0x1234000;
char bootstack[KSTACKSIZE];
void *kmalloc(size_t n) { if (fail_pcb) return NULL; void *p = malloc(n); if (p) pcb_live++; return p; }
void kfree(void *p) { pcb_live--; free(p); }
struct Page *alloc_pages(size_t n) { (void)n; if (fail_stack) return NULL; struct mock_page *p = malloc(sizeof(*p)); if (p) stack_live++; return p ? &p->page : NULL; }
void *page2kva(struct Page *p) { return ((struct mock_page *)p)->stack; }
struct Page *kva2page(void *p) { return &((struct mock_page *)((char *)p - offsetof(struct mock_page, stack)))->page; }
void free_pages(struct Page *p, size_t n) { (void)n; stack_live--; free(p); }
void wakeup_proc(struct proc_struct *p) { p->state = PROC_RUNNABLE; }
void kernel_thread_entry(void) { }
void forkrets(struct trapframe *tf) { (void)tf; }
int cprintf(const char *fmt, ...) { (void)fmt; return 0; }
void __panic(const char *file, int line, const char *fmt, ...) { printf("panic %s:%d %s\n", file, line, fmt); abort(); }
#include "../../kern/process/proc.c"
#define CHECK(x) do { if (!(x)) { printf("FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)
static int thread_fn(void *arg) { return (int)(uintptr_t)arg; }
int main(void) {
 proc_init();
 CHECK(current == idleproc && idleproc->pid == 0 && idleproc->kstack == (uintptr_t)bootstack);
 CHECK(initproc == find_proc(1) && initproc->state == PROC_RUNNABLE && initproc->parent == idleproc);
 CHECK(initproc->pgdir == boot_pgdir_pa && initproc->tf->gpr.a0 == 0);
 CHECK(initproc->tf->gpr.sp == initproc->kstack + KSTACKSIZE && (initproc->tf->gpr.sp & 15) == 0);
 CHECK(initproc->context.ra == (uintptr_t)forkret && initproc->context.sp == (uintptr_t)initproc->tf);
 CHECK(initproc->tf->epc == (uintptr_t)kernel_thread_entry);
 CHECK((initproc->tf->status & (SSTATUS_SPP | SSTATUS_SPIE | SSTATUS_SIE)) == (SSTATUS_SPP | SSTATUS_SPIE));
 int pid = kernel_thread(thread_fn, (void *)42, 0);
 struct proc_struct *child = find_proc(pid);
 CHECK(pid == 2 && child && child->state == PROC_RUNNABLE && child->parent == current);
 CHECK(child->tf->gpr.s0 == (uintptr_t)thread_fn && child->tf->gpr.s1 == 42);
 struct trapframe supplied = {0};
 supplied.gpr.a0 = 123;
 int custom_pid = do_fork(0, 0x8000, &supplied);
 struct proc_struct *custom = find_proc(custom_pid);
 CHECK(custom_pid == 3 && custom && custom->tf->gpr.sp == 0x8000);
 CHECK(custom->tf->gpr.a0 == 0 && supplied.gpr.a0 == 123);
 int pcbs = pcb_live, stacks = stack_live, count = nr_process;
 fail_pcb = 1; CHECK(kernel_thread(thread_fn, NULL, 0) == -E_NO_MEM); fail_pcb = 0;
 CHECK(pcb_live == pcbs && stack_live == stacks && nr_process == count);
 fail_stack = 1; CHECK(kernel_thread(thread_fn, NULL, 0) == -E_NO_MEM); fail_stack = 0;
 CHECK(pcb_live == pcbs && stack_live == stacks && nr_process == count);
 nr_process = MAX_PROCESS; CHECK(kernel_thread(thread_fn, NULL, 0) == -E_NO_FREE_PROC);
 CHECK(pcb_live == pcbs && stack_live == stacks && nr_process == MAX_PROCESS);
 printf("process creation and failure paths: PASS\n"); return 0;
}
