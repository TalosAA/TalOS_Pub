#include <IA32/tab_defs.h>

# void JumpToUserCode (void* StackAddress, uint32_t StackLimig, void* usrCodeAddr)
.global JumpToUserCode
.type JumpToUserCode, @function
JumpToUserCode:
  cli
  mov %esp, %ebp

  mov $(USER_DATA_SEL | 0x03), %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  mov 4(%ebp), %eax # user stack
  add 8(%ebp), %eax # calc stackTop
  mov %eax, %esp
  pushl $(USER_DATA_SEL | 0x03)
  pushl %eax
  pushf
  popl %eax
  or $0x200, %eax   # mask EFLAGS to reactivate interrupts after iret
  pushl %eax
  pushl $(USER_CODE_SEL | 0x03)
  pushl 12(%ebp)    # jump address
  iret