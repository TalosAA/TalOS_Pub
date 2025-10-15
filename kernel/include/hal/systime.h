#ifndef _HAL_SYSTIME_H_
#define _HAL_SYSTIME_H_

#include <libk/stdint.h>
#include <libk/time.h>

/* OS SysTick callback */
typedef void SysTickFun_t(uint64_t counter);

/* System Timer set counter */
extern void SetSysTimerFrequency(uint32_t SysTickFreqHZ);

/* Set sys tick callback */
extern void SetSysTickFun(SysTickFun_t* SysTickFunIn);

/* GetSysTimestamp */
extern time_t* GetSysTimestamp(time_t* timestamp);

/* Get system time (ms) */
extern sys_time_t* GetSysTime_ms(sys_time_t* SysTime_ms);

/* Wait (ms) */
extern void Wait_ms(sys_time_t time_ms);

/* Wait (us) */
extern void Wait_us(sys_time_t time_ms);

/* Read 64-bits systime counter */
extern void ReadTimeCounter (uint64_t* t_input, uint64_t* t_out);

#endif