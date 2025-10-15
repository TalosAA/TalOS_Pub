#ifndef _SYS_TIMER_H_
#define _SYS_TIMER_H_

#include <libk/stdint.h>
#include <libk/stddef.h>
#include <coresys/types.h>

void        InitKernelTimers(void);
void        SysTick(void);

bool        SetTimer(uint16_t index, sys_time_t countDown);
bool        StartTimer(uint16_t index);
bool        SetTimerStart(uint16_t index, sys_time_t countDown);
bool        TimerExpired(uint16_t index);

void        Wait_ms(sys_time_t time_ms);

#endif 
