#include "host_runtime.h"

#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define CAP 16384

static char cap_buf[CAP];
static size_t cap_len;
static char panic_buf[512];
static int panic_flag;
static int interrupt_hits;
static int failures;
static sigjmp_buf escape;
static volatile sig_atomic_t jump_reason;
static volatile sig_atomic_t fault_sig;

void host_capture_reset(void) {
    cap_len = 0;
    cap_buf[0] = '\0';
}

const char *host_capture(void) { return cap_buf; }

void host_interrupt_reset(void) { interrupt_hits = 0; }

int host_interrupt_count(void) { return interrupt_hits; }

void host_note_interrupt(void) { interrupt_hits++; }

int host_panic_called(void) { return panic_flag; }

const char *host_panic_message(void) { return panic_buf; }

int cprintf(const char *fmt, ...) {
    char tmp[1024];
    va_list ap;
    int wrote;
    size_t take;

    va_start(ap, fmt);
    wrote = vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (wrote < 0) {
        return wrote;
    }
    take = (size_t)wrote;
    if (take >= sizeof(tmp)) {
        take = sizeof(tmp) - 1;
    }
    if (cap_len + take >= CAP) {
        take = CAP - 1 - cap_len;
    }
    memcpy(cap_buf + cap_len, tmp, take);
    cap_len += take;
    cap_buf[cap_len] = '\0';
    return wrote;
}

void __attribute__((noreturn)) __panic(const char *file, int line, const char *fmt, ...) {
    va_list ap;

    (void)file;
    (void)line;
    panic_flag = 1;
    va_start(ap, fmt);
    vsnprintf(panic_buf, sizeof(panic_buf), fmt, ap);
    va_end(ap);
    jump_reason = HOST_PANIC;
    siglongjmp(escape, 1);
    abort();
}

static void on_fault(int sig, siginfo_t *info, void *context) {
    (void)info;
    (void)context;
    fault_sig = sig;
    jump_reason = HOST_FAULT;
    siglongjmp(escape, 1);
}

int host_invoke(void (*fn)(void *), void *arg) {
    struct sigaction action;
    struct sigaction old_segv;
    struct sigaction old_bus;

    memset(&action, 0, sizeof(action));
    action.sa_sigaction = on_fault;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGSEGV, &action, &old_segv);
    sigaction(SIGBUS, &action, &old_bus);

    panic_flag = 0;
    panic_buf[0] = '\0';
    fault_sig = 0;
    jump_reason = HOST_OK;
    if (sigsetjmp(escape, 1) != 0) {
        sigaction(SIGSEGV, &old_segv, NULL);
        sigaction(SIGBUS, &old_bus, NULL);
        return (int)jump_reason;
    }
    fn(arg);
    sigaction(SIGSEGV, &old_segv, NULL);
    sigaction(SIGBUS, &old_bus, NULL);
    if (panic_flag) {
        return HOST_PANIC;
    }
    return HOST_OK;
}

const char *host_fault_name(void) {
    if (fault_sig == SIGBUS) {
        return "SIGBUS";
    }
    return "SIGSEGV";
}

static void *map_page(int prot) {
    long page = sysconf(_SC_PAGESIZE);
    void *mem;

    if (page < 4096) {
        page = 4096;
    }
    mem = mmap(NULL, (size_t)page, prot, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (mem == MAP_FAILED) {
        return NULL;
    }
    return mem;
}

void *host_page_write(void) { return map_page(PROT_READ | PROT_WRITE); }

void *host_page_guard(void) { return map_page(PROT_NONE); }

void host_pass(const char *name) { printf("通过 %s\n", name); }

void host_fail_hex(const char *name, unsigned long long actual) {
    failures++;
    printf("错误 %s: 0x%llx\n", name, actual);
}

void host_fail_text(const char *name, const char *actual) {
    char shown[240];
    size_t i = 0;

    failures++;
    if (actual == NULL || actual[0] == '\0') {
        printf("错误 %s: (空)\n", name);
        return;
    }
    while (actual[i] != '\0' && i + 1 < sizeof(shown)) {
        char ch = actual[i];
        if (ch == '\n' || ch == '\r' || ch == '\t') {
            ch = ' ';
        }
        shown[i] = ch;
        i++;
    }
    shown[i] = '\0';
    printf("错误 %s: %s\n", name, shown);
}

int host_failure_count(void) { return failures; }

int host_has(const char *hay, const char *needle) {
    if (hay == NULL || needle == NULL) {
        return 0;
    }
    return strstr(hay, needle) != NULL;
}

int host_number_after(const char *text, const char *marker, unsigned long long *out) {
    const char *at;
    char *end = NULL;

    if (text == NULL || marker == NULL || out == NULL) {
        return 0;
    }
    at = strstr(text, marker);
    if (at == NULL) {
        return 0;
    }
    at += strlen(marker);
    while (*at != '\0') {
        if (at[0] == '0' && (at[1] == 'x' || at[1] == 'X')) {
            *out = strtoull(at, &end, 16);
            return end != at;
        }
        if (*at >= '0' && *at <= '9') {
            *out = strtoull(at, &end, 10);
            return end != at;
        }
        at++;
    }
    return 0;
}

int host_hex_after(const char *text, const char *marker, unsigned long long *out) {
    const char *at;
    char *end = NULL;

    if (text == NULL || marker == NULL || out == NULL) {
        return 0;
    }
    at = strstr(text, marker);
    if (at == NULL) {
        return 0;
    }
    at += strlen(marker);
    *out = strtoull(at, &end, 16);
    return end != at;
}

void host_join(char *dst, unsigned cap, const char *a, const char *b) {
    if (b != NULL && b[0] != '\0') {
        snprintf(dst, cap, "%s-%s", a, b);
    } else {
        snprintf(dst, cap, "%s", a);
    }
}

void host_name_cause(char *dst, unsigned cap, const char *prefix, unsigned cause) {
    snprintf(dst, cap, "%s-%u", prefix, cause);
}
