#ifndef _HAL_PLAT_H_
#define _HAL_PLAT_H_

/* System Stop */
extern void StopSystem(void);

/* System Reset */
extern void ResetSystem(void);

/* Requests a logic addess mapping for a physical addresses block */
//TODO PAGE SIZE DEPENDENCY
#define KMEM_PAGE_SIZE    (1024)
#define KMEM_READ         ((bool)FALSE)
#define KMEM_RW           ((bool)TRUE)
#define KMEM_CACHEABLE    ((bool)TRUE)
#define KMEM_UNCACHEABLE  ((bool)FALSE)

extern void* mem_map(void* phyAddress, void* logiAddress, size_t memsize,
                     bool isWritable, bool isCacheble, size_t* mappedSize);

extern void mem_unmap(void* logiAddress, size_t memsize);

/* 
 * This function can be used inside the
 * loops to inform the platform that an
 * active wait loop is performing
 */
extern void SpinDelay(void);

/* Atomically sets the first bit and waits the clearing
   if the bit is already set. */
extern void AtomicTestSetAndWait(uint32_t* lock);

#endif