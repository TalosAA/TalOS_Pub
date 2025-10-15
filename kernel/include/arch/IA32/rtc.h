#ifndef _IA32_RTC_H_
#define _IA32_RTC_H_

#include <libk/time.h>

void RTC_Init(void);
bool RTC_GetTimestamp(time_t* timestamp);

#endif
