#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <libk/stdint.h>
#include <libk/stddef.h>
#include <libk/limits.h>
#include <hal/init.h>
#include <hal/systime.h>
#include <coresys/osconfig.h>
#include <coresys/types.h>
#include <coresys/vfs.h>

/* Operating System Data Section */
#define __OS_DATA             __attribute__((section(".OS_DATA")))

typedef struct {
  /* TBD: define other OS */
  SysConf_t           SystemConfig;
  time_t              timestamp;
  sys_time_t          SysTickPeriod_ms;
  sys_time_t          SysTime_ms;
  timer_block_t       TimeSliceTimer;
  timer_block_t       TimerBlocks[OS_TIMERS_N];
} os_data_t;


#endif
