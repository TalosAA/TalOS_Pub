#include <IA32/memdefs.h>
#include <IA32/init_defs.h>

.section .text

# This functions enables the paging by setting
# the CR3 register of the CPU
# C-prototype: void paging_enable(void)
.global paging_enable
.type paging_enable, @function
paging_enable:
  pushl  %ebp
  movl   %esp, %ebp
  mov $(page_directory), %ecx
  mov %ecx, %cr3
  mov %cr0, %eax
  or $0x80000000, %eax
  mov %eax, %cr0
  popl %ebp
  ret   

# This functions re-sets the paging after a changing
# to the page tables or directories
# C-prototype: void paging_reset(void)
.global paging_reset
.type paging_reset, @function
paging_reset:
  cli
  pushl  %ebp
  movl   %esp, %ebp
  pushl  %ecx
  pushl  %eax
  # Invalidate TLB
  mov %cr3, %ecx
  mov %ecx, %cr3
  # Reset Paging
  mov %cr0, %eax
  or $0x80000000, %eax
  mov %eax, %cr0
  popl %eax
  popl %ecx
  popl %ebp
  sti
  ret
