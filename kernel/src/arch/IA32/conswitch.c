#include <IA32/tables.h>
#include <IA32/sysinit.h>

//TODO: limit check to be managed
void SetSysCallsStackAddress(uint32_t CoreID, void* address, uint32_t limit) {
  Set_Kernel_TSS_StackPointer(CoreID, (uint32_t)(address + limit));
}

void SetUserStackAddress(void* address, uint32_t limit) {
  uint32_t stackTop = ((uint32_t)address) + limit;
  __asm__ ("movl %0, %%esp" : :"r"(stackTop));
}