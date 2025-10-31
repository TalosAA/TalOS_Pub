#ifndef _STDDEF_H_
#define _STDDEF_H_

#include <libk/stdint.h>

/* NULL Pointer */
#ifndef NULL
  #define NULL ((void*)0)
#endif

#define UINTPTR_NULL  ((uintptr_t)0)

/* Booleans Type */
typedef unsigned char  bool;
#define TRUE        1
#define FALSE       0
#define true        1
#define false       0

typedef enum {
  TRUE_SAFE = 0xA55A, 
  FALSE_SAFE = 0x5AA5
} safe_bool_t;

/* Size Types */
typedef unsigned long size_t;
typedef long ssize_t;
typedef int rsize_t;

/* Error Type */
typedef int errno_t;

/* Unused attribute */
#define __is_unused __attribute__((unused))

/* Packed attribute */
#define __is_packed __attribute__ ((packed, aligned(1)))

#endif