#ifndef _IA32_PAGING_H_
#define _IA32_PAGING_H_

#include <IA32/memdefs.h>
#include <libk/stdint.h>
#include <libk/stddef.h>

/* Paging Definitions */
#define PAGE_DIR_LEN (1024)
#define PAGE_TAB_LEN (1024)

void paging_enable(void);

void page_mapping(uint32_t* pPageTable, uint32_t phyAddrStart,
                  uint32_t areaSize, uint32_t flags);

void paging_reset(void);

void InitPaging(void);

void* kmem_map(void* phyaddress, size_t memsize, bool isWritable,
               bool isCacheble);

void set_page_table(uint32_t* pPageTable, void* logiAddress, bool isWritable,
                    bool isCacheble, bool isUserTable);

void unset_page_table(void* logiAddress);

void change_page_table_attributes(void* logiAddress, bool isPresent,
                                  bool isWritable, bool isCacheble,
                                  bool isUserland);

void* mem_map(void* phyAddress, void* logiAddress, size_t memsize,
              bool isWritable, bool isCacheble, size_t* mappedSize);

void mem_unmap(void* logiAddress, size_t memsize);

bool is_mapped(void* logiAddress, size_t size);

#endif