#ifndef HOST_RUNTIME_H
#define HOST_RUNTIME_H

#define HOST_OK 0
#define HOST_PANIC 1
#define HOST_FAULT 2

void host_capture_reset(void);
const char *host_capture(void);
void host_interrupt_reset(void);
int host_interrupt_count(void);
void host_note_interrupt(void);
int host_panic_called(void);
const char *host_panic_message(void);

int host_invoke(void (*fn)(void *), void *arg);
const char *host_fault_name(void);
void *host_page_write(void);
void *host_page_guard(void);

int host_has(const char *hay, const char *needle);
int host_number_after(const char *text, const char *marker, unsigned long long *out);
int host_hex_after(const char *text, const char *marker, unsigned long long *out);
void host_join(char *dst, unsigned cap, const char *a, const char *b);
void host_name_cause(char *dst, unsigned cap, const char *prefix, unsigned cause);

void host_pass(const char *name);
void host_fail_hex(const char *name, unsigned long long actual);
void host_fail_text(const char *name, const char *actual);
int host_failure_count(void);

#endif
