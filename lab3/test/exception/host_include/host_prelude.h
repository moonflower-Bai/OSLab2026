/* 宿主机编译器拒绝 libs/atomic.h 里的 RISC-V amo 与 "+A" 约束。
 * 这里用 C 实现同一组位操作，并占住头文件保护宏，避免同一翻译单元再定义一次。
 * 本文件不放在会替代 <atomic.h> 的搜索路径上。 */
#include <defs.h>

#ifndef __LIBS_ATOMIC_H__
#define __LIBS_ATOMIC_H__

static inline void set_bit(int nr, volatile void *addr) {
    volatile unsigned long *word = (volatile unsigned long *)addr;
    int bits = (int)(sizeof(unsigned long) * 8);
    word[nr / bits] |= 1UL << (nr % bits);
}

static inline void clear_bit(int nr, volatile void *addr) {
    volatile unsigned long *word = (volatile unsigned long *)addr;
    int bits = (int)(sizeof(unsigned long) * 8);
    word[nr / bits] &= ~(1UL << (nr % bits));
}

static inline void change_bit(int nr, volatile void *addr) {
    volatile unsigned long *word = (volatile unsigned long *)addr;
    int bits = (int)(sizeof(unsigned long) * 8);
    word[nr / bits] ^= 1UL << (nr % bits);
}

static inline bool test_bit(int nr, volatile void *addr) {
    return ((*(volatile unsigned long *)addr) >> nr) & 1;
}

static inline bool test_and_set_bit(int nr, volatile void *addr) {
    bool old = test_bit(nr, addr);
    set_bit(nr, addr);
    return old;
}

static inline bool test_and_clear_bit(int nr, volatile void *addr) {
    bool old = test_bit(nr, addr);
    clear_bit(nr, addr);
    return old;
}

#endif
