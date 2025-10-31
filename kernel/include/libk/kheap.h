


#include <libk/stddef.h>
#include <libk/stdint.h>

#define MEM_MIN_SIZE  (sizeof(unsigned int))

void InitKernelHeap(void* startAddr, size_t memSize);
void *kmalloc(size_t size);
void *kcalloc(size_t nmemb, size_t size);
void *krealloc(void *ptr, size_t size);
void kfree(void *ptr);

/* Cleenup attribute */
#define __cleenup_kmem __attribute__((cleanup(kfree)))