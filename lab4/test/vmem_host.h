#ifndef LAB4_VMEM_HOST_H
#define LAB4_VMEM_HOST_H

#include <defs.h>
#include <riscv.h>

#define __SBI_H__
#define __LIBS_ATOMIC_H__
#define RISCV_PGSHIFT 12
#define read_csr(reg) 0UL
#define write_csr(reg, value) ((void)(value))

static inline volatile uint_t *vmem_bit_word(int bit, volatile void *address)
{
    return (volatile uint_t *)address + bit / 64;
}

static inline bool test_and_set_bit(int bit, volatile void *address)
{
    uint_t mask = 1ULL << (bit % 64);
    return (__atomic_fetch_or(vmem_bit_word(bit, address), mask, __ATOMIC_RELAXED) & mask) != 0;
}

static inline bool test_and_clear_bit(int bit, volatile void *address)
{
    uint_t mask = 1ULL << (bit % 64);
    return (__atomic_fetch_and(vmem_bit_word(bit, address), ~mask, __ATOMIC_RELAXED) & mask) != 0;
}

static inline void set_bit(int bit, volatile void *address) { test_and_set_bit(bit, address); }
static inline void clear_bit(int bit, volatile void *address) { test_and_clear_bit(bit, address); }

static inline void change_bit(int bit, volatile void *address)
{
    __atomic_fetch_xor(vmem_bit_word(bit, address), 1ULL << (bit % 64), __ATOMIC_RELAXED);
}

static inline bool test_bit(int bit, volatile void *address)
{
    return (__atomic_load_n(vmem_bit_word(bit, address), __ATOMIC_RELAXED) >> (bit % 64)) & 1;
}

void vmem_host_sfence_page(uintptr_t va);
void vmem_host_sfence_all(void);

#endif
