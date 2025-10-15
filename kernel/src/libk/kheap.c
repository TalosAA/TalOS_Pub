#include <libk/stddef.h>
#include <libk/stdint.h>
#include <libk/kheap.h>

//TODO: questo modulo è solo un semplice stub ed è completamente da rivedere

static void* HeapMem = NULL;
static size_t HeapMemSize = 0;
static uint8_t* CurrentPointer = NULL;

void InitKernelHeap(void* startAddr, size_t memSize) {
  HeapMem =\
  (void*)((((uint32_t)startAddr + MEM_MIN_SIZE - 1)/MEM_MIN_SIZE)*MEM_MIN_SIZE);
  CurrentPointer = HeapMem;
  HeapMemSize = memSize;
}

void *kmalloc(size_t size) {
  size_t sizeToAlloc = ((size + MEM_MIN_SIZE - 1)/MEM_MIN_SIZE)*MEM_MIN_SIZE;
  void* returnAddr = NULL;
  size_t nextAddrInt = ((size_t)CurrentPointer + sizeToAlloc);
  if(nextAddrInt <= (HeapMemSize + (size_t)HeapMem)) {
    returnAddr = CurrentPointer;
    CurrentPointer = (void*)nextAddrInt;
  }
  return returnAddr;
}

void kfree(void *ptr) {
  //TODO
}