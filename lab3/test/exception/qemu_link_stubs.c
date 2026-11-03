#include <console.h>
#include <sbi.h>
#include <stdio.h>
#include <trap.h>

void cons_init(void) {}

void idt_init(void) {}

int cprintf(const char *fmt, ...) {
    (void)fmt;
    return 0;
}

void sbi_shutdown(void) {}

void trap(struct trapframe *tf) { (void)tf; }

void __wrap_kern_init(void);

void qemu_link_keep(void) { __wrap_kern_init(); }
