#ifndef _HAL_INIT_H_
#define _HAL_INIT_H_

#include <libk/stdint.h>
#include <libk/stddef.h>

/* System configuration for the kernel */
typedef struct {
  #define CPU_INFO_STRING_LEN   (50)
  char     CPU_InfoString[CPU_INFO_STRING_LEN];
  uint16_t CoresNum;
  uint64_t EndOfMemory;
} SysConf_t;

/* Initialize and enable paging */
extern void InitPaging(void);

/* Initialize the system */
extern void InitSys(void);

/* Get the system configuration */
extern void GetSysConfiguration(SysConf_t* SysConf);

#endif