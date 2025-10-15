#include <libk/stdint.h>
#include <hal/bootloader.h>
#include <hal/mmap.h>
#include <IA32/multiboot.h>

extern multiboot_info_t *multiboot_info;
size_t memAreasNum;
mem_area_t memAreas[MAX_MEM_AREAS];

void bl_GetModules(bootloader_module_t** modules, uint32_t* nModules) {
  multiboot_module_t* mbmodule = (multiboot_module_t*)multiboot_info->mods_addr;
  uint32_t i = 0;
  
  *modules = kmalloc(multiboot_info->mods_count * sizeof(bootloader_module_t));
  if(*modules == NULL) {
    //TODO PANIC
    while (1);
  }

  for(i = 0; i < multiboot_info->mods_count; i++) {
    (*modules)[i].mod_start = mbmodule[i].mod_start;
    (*modules)[i].mod_size = mbmodule[i].mod_end - mbmodule[i].mod_start;
  }

  *nModules = multiboot_info->mods_count;
}

void GetMemoryMap(mem_map_t* memMap) {
  uint32_t i = 0;
  multiboot_memory_map_t* mbmm;

  /* Get memory mapping from GRUB */
  if(multiboot_info != NULL) {
    /* check if memory mapping is valid */
    if(!(multiboot_info->flags >> 6 & 0x1)) {
        //TODO PANIC
        while(1);
    }

    memAreasNum = 0;

    for(i = 0;
        i < multiboot_info->mmap_length;
        i += sizeof(multiboot_memory_map_t)) 
    {
      mbmm =\
          (multiboot_memory_map_t*) ((uint32_t)multiboot_info->mmap_addr + i);

      if(mbmm->type == MULTIBOOT_MEMORY_AVAILABLE) {
        memAreas[memAreasNum].start = (uintptr_t)mbmm->addr;
        memAreas[memAreasNum].length = (uintptr_t)mbmm->len;
        memAreasNum++;
      }
    }
    memMap->areas = memAreas;
    memMap->areasNum = memAreasNum;
  } else {
    //TODO PANIC
    while(1);
  }

}