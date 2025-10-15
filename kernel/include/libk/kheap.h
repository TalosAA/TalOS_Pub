


#include <libk/stddef.h>
#include <libk/stdint.h>

#define MEM_MIN_SIZE  (sizeof(unsigned int))

void InitKernelHeap(void* startAddr, size_t memSize);
void *kmalloc(size_t size);
void kfree(void *ptr);