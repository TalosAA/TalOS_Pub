#include <IA32/paging.h>
#include <IA32/utils.h>
#include <libk/string.h>

/**
 * Paging flags
 */
#define PAG_IS_PRESENT    (0x00000001)
#define PAG_CACHE_DISABLE (0x00000010)
#define PAG_IS_RW         (0x00000002)
#define PAG_IS_NOT_SUPERV (0x00000004)

/**
 * Linker Script Variables
 */
extern char _ker_page_table_size;
extern char _ker_fixed_end;
extern char _ker_ro_area_start;
extern char _ker_ro_area_size;
extern char _ker_ro_area_end;
extern char _ker_data_start;
extern char _ker_data_size;

/**
 *  Page directory and page tables of the kernel
 */
uint32_t page_directory[PAGE_DIR_LEN] __ALIGN_PAGE_SIZE;
uint32_t kernel_page_tables[KER_PAGES_NUM] __ALIGN_PAGE_SIZE;

uint32_t mmap_io_page_tables[KER_MMAP_IO_N_PAGES] __ALIGN_PAGE_SIZE;

/**
 * First page of the free memory of the kernel
 */
uint32_t kernel_first_free_vir_page = 0;

void page_mapping(uint32_t* pPageTable, uint32_t phyAddrStart,
                  uint32_t areaSize, uint32_t flags) {
  phyAddrStart = PAGE_ADDRESS(phyAddrStart);
  for (; areaSize > 0;
       phyAddrStart += PAGE_SIZE, areaSize -= PAGE_SIZE, pPageTable++) {
    *pPageTable = phyAddrStart | flags;
  }
}

bool is_mapped(void* logiAddress, size_t size) {
  uintptr_t pageAddr = NULL;
  size_t sizePag = PAGE_ALIGN_CEIL(size) / PAGE_SIZE;
  uint32_t* pageTable = NULL;
  uint32_t i = 0;
  uint32_t k = 0;
  bool ret = true;

  for (k = 0; k < sizePag; k++) {
    pageAddr = PAGE_ALIGN_FLOOR((uintptr_t)logiAddress) + k * PAGE_SIZE;
    i = GET_PAG_DIR_INDEX(pageAddr);
    if ((page_directory[i] & PAG_IS_PRESENT) == PAG_IS_PRESENT) {
      pageTable = (uint32_t*)PAGE_ADDRESS(page_directory[i]);
      i = GET_PAG_TAB_INDEX(pageAddr);
      if ((pageTable[i] & PAG_IS_PRESENT) != PAG_IS_PRESENT) {
        ret = false;
        break;
      }
    } else {
      ret = false;
      break;
    }
  }

  return ret;
}

void* mem_map(void* phyAddress, void* logiAddress, size_t memsize,
              bool isWritable, bool isCacheble, size_t* mappedSize) {
  uint32_t index = 0;
  uint32_t k = 0;
  void* retAddr = NULL;
  uint32_t flags = PAG_IS_PRESENT;
  uintptr_t logiPageAddressCurr = 0;
  uintptr_t logiPageAddressStart = PAGE_ALIGN_FLOOR((uintptr_t)logiAddress);
  uint32_t pagesNum = PAGE_ALIGN_CEIL(memsize) / PAGE_SIZE;
  uint32_t pageTabs = pagesNum / PAGE_TAB_ENTRIES;
  uint32_t* pPageTable = NULL;

  /* initialize output */
  if (mappedSize != NULL) {
    *mappedSize = 0;
  }

  if (isWritable) {
    flags |= PAG_IS_RW;
  }

  if (!isCacheble) {
    flags |= PAG_CACHE_DISABLE;
  }

  /* Check if page tables are present */
  index = GET_PAG_DIR_INDEX(logiPageAddressStart);
  k = index;
  do {
    if ((page_directory[k++] & PAG_IS_PRESENT) != PAG_IS_PRESENT) {
      break;
    }
  } while (k < (index + pageTabs));

  /* Pages mapping */
  if (k >= (index + pageTabs)) {
    for (k = 0; k < pagesNum; k++) {
      logiPageAddressCurr = logiPageAddressStart + k * PAGE_SIZE;
      index = GET_PAG_DIR_INDEX(logiPageAddressCurr);
      pPageTable = (void*)PAGE_ADDRESS(page_directory[index]);
      index = GET_PAG_TAB_INDEX(logiPageAddressCurr);
      pPageTable[index] =
          (PAGE_ALIGN_FLOOR((uintptr_t)phyAddress) + k * PAGE_SIZE) | flags;
    }

    /* prepare outputs */
    if (mappedSize != NULL) {
      *mappedSize = pagesNum * PAGE_SIZE;
    }
    retAddr = (void*)logiPageAddressStart;
  }

  return retAddr;
}

void mem_unmap(void* logiAddress, size_t memsize) {
  uint32_t index = 0;
  uint32_t k = 0;
  uintptr_t logiPageAddressCurr = 0;
  uintptr_t logiPageAddressStart = PAGE_ALIGN_FLOOR((uintptr_t)logiAddress);
  uint32_t pagesNum = PAGE_ALIGN_CEIL(memsize) / PAGE_SIZE;
  uint32_t* pPageTable = NULL;

  /* Pages unmapping */
  for (k = 0; k < pagesNum; k++) {
    logiPageAddressCurr = logiPageAddressStart + k * PAGE_SIZE;
    index = GET_PAG_DIR_INDEX(logiPageAddressCurr);
    if ((pPageTable = (void*)PAGE_ADDRESS(page_directory[index])) != NULL) {
      index = GET_PAG_TAB_INDEX(logiPageAddressCurr);
      pPageTable[index] = 0;
    }
  }
}

void set_page_table(uint32_t* pPageTable, void* logiAddress, bool isWritable,
                    bool isCacheble, bool isUserTable) {
  uint32_t flags = PAG_IS_PRESENT;

  if (isWritable) {
    flags |= PAG_IS_RW;
  }

  if (!isCacheble) {
    flags |= PAG_CACHE_DISABLE;
  }

  if (isUserTable) {
    flags |= PAG_IS_NOT_SUPERV;
  }

  page_directory[GET_PAG_DIR_INDEX((uintptr_t)logiAddress)] =
      ((uint32_t)pPageTable) | flags;

}

void unset_page_table(void* logiAddress) {
  page_directory[GET_PAG_DIR_INDEX((uintptr_t)logiAddress)] = 0;
}

void change_page_table_attributes(void* logiAddress, bool isPresent,
                                  bool isWritable, bool isCacheble,
                                  bool isUserland) {
  uint32_t index = GET_PAG_DIR_INDEX((uintptr_t)logiAddress);

  page_directory[index] &= 0xFFFFF000;

  if(isPresent) {
    page_directory[index] |= PAG_IS_PRESENT;
  }

  if (isWritable) {
    page_directory[index] |= PAG_IS_RW;
  }

  if (!isCacheble) {
    page_directory[index] |= PAG_CACHE_DISABLE;
  }

  if (isUserland) {
    page_directory[index] |= PAG_IS_NOT_SUPERV;
  }

}

void InitPaging(void) {
  uint32_t i = 0;
  uint32_t k = 0;

  /* Init Page Directory and kernel page tables */
  for (i = 0; i < (PAGE_DIR_LEN / sizeof(uint32_t)); i++) {
    page_directory[i] = 0;
  }

  /* Init Page Directory and kernel page tables */
  for (i = 0; i < (KER_PAGES_NUM / sizeof(uint32_t)); i++) {
    kernel_page_tables[i] = 0;
  }

  /* Init Page Directory and kernel page tables */
  for (i = 0; i < (KER_MMAP_IO_N_PAGES / sizeof(uint32_t)); i++) {
    mmap_io_page_tables[i] = 0;
  }

  /* Mapping of the kernel */
  page_mapping((uint32_t*)kernel_page_tables, KER_MEM_START_ADDR,
               (uint32_t)&_ker_ro_area_start, PAG_IS_PRESENT | PAG_IS_RW);

  /* Set Kernel .text and .rodata read only */
  i = (uint32_t)(&_ker_ro_area_start);
  page_mapping((uint32_t*)&kernel_page_tables[i / PAGE_SIZE], i,
               PAGE_ALIGN_FLOOR(&_ker_ro_area_size), PAG_IS_PRESENT);

  /* Mapping of the kernel */
  i = (uint32_t)(&_ker_ro_area_end);
  page_mapping((uint32_t*)&kernel_page_tables[i / PAGE_SIZE], i,
               PAGE_ALIGN_FLOOR(&_ker_data_size), PAG_IS_PRESENT | PAG_IS_RW);

  kernel_first_free_vir_page =
      (i / PAGE_SIZE) + PAGE_ALIGN_FLOOR(&_ker_data_size) / PAGE_SIZE;

  /* Map the page tables of the kernel in the page directory */
  for (i = 0; i < KER_TABS; i++) {
    page_directory[i] = ((uint32_t)&kernel_page_tables[i * PAGE_TAB_LEN]) |
                        PAG_IS_PRESENT | PAG_IS_RW;
  }

  k = 0;
  i = GET_PAG_DIR_INDEX(KER_MMAP_IO_ADDR_START);
  for (; i < (GET_PAG_DIR_INDEX(KER_MMAP_IO_ADDR_START) +
              KER_MMAP_IO_N_PAGES / PAGE_TAB_ENTRIES);
       i++, k++) {
    page_directory[i] = ((uint32_t)&mmap_io_page_tables[k * PAGE_TAB_LEN]) |
                        PAG_IS_PRESENT | PAG_IS_RW;
  }

  paging_enable();
}
