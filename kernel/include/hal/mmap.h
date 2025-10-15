#ifndef _HAL_MMAP_H_
#define _HAL_MMAP_H_

#include <libk/stdint.h>
#include <libk/stddef.h>

typedef struct {
  uintptr_t start;
  size_t length;
} mem_area_t;

typedef struct {
  size_t areasNum;
  mem_area_t* areas;
} mem_map_t;

void GetMemoryMap(mem_map_t* memMap);

#endif