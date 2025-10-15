#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_

/**
 *  OS Basic Information
 **/
#define OS_NAME               "TalOS"
#define OS_VERSION            "0.0.1"

/**
 *  Timers Configuration
 **/
/* Sys Tick configuration */
#define SYS_TICK_1000_HZ

/* Number of software timers */
#define OS_TIMERS_N           (20)      /* Number of timers blocks */

/* Scheduling timeslice definition */
#define OS_TIME_SLICE         (20)      /* ms */

/* Kernel HEAP Size */
#define KER_HEAP_SIZE         (0x20000)

/**
 * Resources configuration
 **/
/* System resources in fio format */
#define SYS_RESOURCES_NUM     (255)

/* <WARNING> DO NOT EDIT THIS SECTION */
#define SYS_TICK_MAX_HZ    (1000)       /* T = 1 ms */
#ifdef SYS_TICK_1000_HZ
  #define SYS_TICK_FREQ_HZ (1000)       /* T = 1 ms */
#elif defined(SYS_TICK_500_HZ)
  #define SYS_TICK_FREQ_HZ (500)        /* T = 2 ms */
#elif defined(SYS_TICK_250_HZ)
  #define SYS_TICK_FREQ_HZ (250)        /* T = 4 ms */
#elif defined(SYS_TICK_125_HZ)
  #define SYS_TICK_FREQ_HZ (125)        /* T = 8 ms */
#elif defined(SYS_TICK_50_HZ)
  #define SYS_TICK_FREQ_HZ (50)         /* T = 20 ms */
#else
  #error "Define a SYS_TICK frequency undefined or unsupported."
#endif

#define SYS_TICK_INCREMENT_MS  (SYS_TICK_MAX_HZ/SYS_TICK_FREQ_HZ)

#endif