#ifndef _IA32_MEMDEFS_H_
#define _IA32_MEMDEFS_H_

/* Number of pages per page table */
#define PAGE_TAB_ENTRIES      (0x400)

/* Size of a memory page (4 kiB)*/
#define PAGE_SIZE             (0x1000)

/* Page Alignment */
#define __ALIGN_PAGE_SIZE     __attribute__((aligned(PAGE_SIZE)))

/* Get page table index */
#define GET_PAG_TAB_INDEX(ADDR)   (((ADDR)/PAGE_SIZE) % PAGE_TAB_ENTRIES)

/* Get dir table index */
#define GET_PAG_DIR_INDEX(ADDR)   (((ADDR)/PAGE_SIZE)/PAGE_TAB_ENTRIES)

/* Get page address */
#define GET_PAG_PTR_BY_INDEX(INDEX) ((INDEX) * PAGE_SIZE)

#define PAGE_ALIGN_CEIL(_value)\
                    ((((uint32_t)(_value) + (PAGE_SIZE-1))/PAGE_SIZE)*PAGE_SIZE)
#define PAGE_ALIGN_FLOOR(_value)\
                    ((((uint32_t)(_value))/PAGE_SIZE)*PAGE_SIZE)

/* Mask to get the page address */
#define PAGE_ADDRESS(_addr) (0xfffff000 & (_addr))

/* AP trampoline code address */
#define AP_TRAM_START         (0x8000)

/**
 * BIOS physical addresses
 */
#define BIOS_START_ADDR       ((uint8_t*)0x000E0000)
#define BIOS_END_ADDR         ((uint8_t*)0x000FFFFF)

/* Kernel STACK Size (16 kiB) */
#define CORE_STACK_SIZE       (0x2000)

/* Kernel reserved virtual memory (first GiB) */
#define KER_MEM_START_ADDR        (0)
#define KER_MEM_MAX_SIZE          (0x40000000)
#define KER_MEM_END_ADDR          (KER_MEM_START_ADDR + KER_MEM_MAX_SIZE)

/* Kernel Pages */
#define KER_PAGES_NUM             (KER_MEM_MAX_SIZE/PAGE_SIZE)
#define KER_TABS                  (KER_PAGES_NUM/PAGE_TAB_ENTRIES)

/* Memory mapping I/O addresses (last 28 MiB) */
#define KER_MMAP_IO_ADDR_START  PAGE_ALIGN_FLOOR(0xFE000000)
#define KER_MMAP_IO_ADDR_END    PAGE_ALIGN_FLOOR(0xFFFFFFFF)
#define KER_MMAP_IO_N_PAGES \
((PAGE_ALIGN_FLOOR(KER_MMAP_IO_ADDR_END-KER_MMAP_IO_ADDR_START))/PAGE_SIZE)

#endif
