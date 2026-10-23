#ifndef __TEST_LAB2_P2_BLACKBOX_H__
#define __TEST_LAB2_P2_BLACKBOX_H__

#include <defs.h>

/* Run the Lab2 P2 Best-Fit black-box suite. Called once at the end of
 * pmm_init() in a temporary, injected copy of lab2. P2_CASE selects one case
 * by number (1-based); 0 runs every case in order. */
void test_p2_blackbox(void);

#endif /* !__TEST_LAB2_P2_BLACKBOX_H__ */
