#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int cprintf(const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    int result = vprintf(format, arguments);
    va_end(arguments);
    fflush(stdout);
    return result;
}

_Noreturn void __panic(const char *file, int line, const char *format, ...)
{
    va_list arguments;
    printf("panic %s:%d: ", file, line);
    va_start(arguments, format);
    vprintf(format, arguments);
    va_end(arguments);
    putchar('\n');
    exit(1);
}

void intr_enable(void) {}
void intr_disable(void) {}
