#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <coresys/types.h>

typedef uint32_t spinlock_t;

/* Initialize spinlock */
void spinlock_init(spinlock_t *p);

/* Acquire spinlock */
void spinlock_acquire(spinlock_t *p);

/* Release spinlock */
void spinlock_release(spinlock_t *p);

/* Checks if the spinlock is acquired (without acquiring) */
bool spinlock_is_acquired(spinlock_t *p);

#endif
