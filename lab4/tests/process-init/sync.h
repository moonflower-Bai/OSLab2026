#ifndef TEST_SYNC_H
#define TEST_SYNC_H
#include <defs.h>
#define SSTATUS_SIE 0x02
#define SSTATUS_SPIE 0x20
#define SSTATUS_SPP 0x100
#define read_csr(x) (SSTATUS_SIE)
#define local_intr_save(x) do { (x) = 1; } while (0)
#define local_intr_restore(x) do { (void)(x); } while (0)
#endif
