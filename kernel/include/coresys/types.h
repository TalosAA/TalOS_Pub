#ifndef _KER_TYPES_H_
#define _KER_TYPES_H_

#include <libk/time.h>
#include <libk/stddef.h>
#include <libk/stdint.h>
#include <coresys/spinlock.h>

/* Return Types */
typedef int ret_t;

#define RET_T_OK    (0)
#define RET_T_NOK   (-1)

typedef struct {
    bool        Started;
    bool        Expired;
    uint32_t    CountDown;
} timer_block_t;

#endif
