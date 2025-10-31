#include <libk/stddef.h>
#include <libk/stdint.h>
#include <libk/kheap.h>
#include <libk/string.h>

//TODO: this module is only a stub, please ignore it :D

// Next is used to prevent the realloc stub to read
// an unpaginated memory :S :S :S
#define KHEAP_MAGIC_VAL       ((size_t)"NEXT")

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
  *((size_t*)nextAddrInt) = KHEAP_MAGIC_VAL;
  nextAddrInt += sizeof(size_t);
  return returnAddr;
}

void *kcalloc(size_t nmemb, size_t size) {
  return kmalloc(nmemb * size);
}

void *krealloc(void *ptr, size_t size) {
  void* newPtr = NULL;
  size_t count = size;
  void* ptrCount = ptr;
  size_t oldSize = 0;

  //find magic val
  while(count != 0 && *((size_t*)ptrCount) != KHEAP_MAGIC_VAL) {
    count--;
    oldSize++;
    ptrCount++;
  }
  oldSize--;

  if(oldSize > size) {
    oldSize = size;
  }

  if(size > 0) {
    newPtr = kmalloc(size);
    memcpy(newPtr, ptr, oldSize);
  }
  kfree(ptr);
  return newPtr;
}

void kfree(void *ptr) {
  //TODO
}