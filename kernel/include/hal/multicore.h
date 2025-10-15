#ifndef _HAL_MULTI_CORE_H_
#define _HAL_MULTI_CORE_H_

#include <libk/stddef.h>
#include <libk/stdint.h>

extern uint32_t GetCoreID(void);
extern uint32_t GetNumberOfCores(void);

#endif