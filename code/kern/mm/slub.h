#ifndef __KERN_MM_SLUB_H__
#define __KERN_MM_SLUB_H__

#include <defs.h>

#define SLUB_ALIGNMENT 16
#define SLUB_MAX_SMALL 2048

/* Call once after pmm_init(). This lab implementation requires serialized use. */
void slub_init(void);

/* At least size writable bytes, aligned to SLUB_ALIGNMENT; contents unspecified.
 * Zero size, overflow and out of memory return NULL without changing live objects.
 */
void *kmalloc(size_t size);

/* NULL is a no-op. Otherwise ptr must be a live, exact kmalloc return value.
 * Detected invalid/double frees panic. A reused address has no generation tag.
 */
void kfree(void *ptr);

/* Independent extension checks; supplied physical-page checks stay unchanged. */
void slub_check(void);
void slub_stress(size_t operations, unsigned int seed);

#endif /* !__KERN_MM_SLUB_H__ */
