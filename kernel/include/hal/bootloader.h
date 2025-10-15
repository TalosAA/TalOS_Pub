#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include <libk/stdint.h>
#include <libk/stddef.h>

#define MAX_MEM_AREAS     (50)

struct bootloader_module {
  void* mod_start;
  size_t mod_size;
};

typedef struct bootloader_module bootloader_module_t;

void bl_GetModules(bootloader_module_t** modules, uint32_t* nModules);

#endif