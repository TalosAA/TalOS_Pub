#include <coresys/spinlock.h>
#include <hal/platform.h>

#define SPINLOCK_BIT_LOCK             (1)

void spinlock_init(spinlock_t *p) {
  *p = ~SPINLOCK_BIT_LOCK & (*p);
}

void spinlock_acquire(spinlock_t *p)
{
    AtomicTestSetAndWait(p);
}

bool spinlock_is_acquired(spinlock_t *p){
    register bool ret = FALSE;
    if(((*p) & SPINLOCK_BIT_LOCK) == SPINLOCK_BIT_LOCK){
      ret = TRUE;
    }
    SpinDelay();
    return ret;
}

void spinlock_release(spinlock_t *p)
{
    *p = ~SPINLOCK_BIT_LOCK & (*p);
}

