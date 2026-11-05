#ifndef HOST_CSR_H
#define HOST_CSR_H

/* 宿主机没有 CSR 指令。这些宏只让 trap.c 里的 idt_init 能编译。 */
#define read_csr(reg) (0UL)
#define write_csr(reg, val) ((void)(val))
#define swap_csr(reg, val) (0UL)
#define set_csr(reg, bit) (0UL)
#define clear_csr(reg, bit) (0UL)

#endif
