#include <IA32/memdefs.h>
#include <IA32/init_defs.h>
#include <IA32/tab_defs.h>

.section .text
.code16
.global _trampolineAP
.type _trampolineAP, @function
_trampolineAP:
  # Disable interrupt during BOOT sequence
  cli
  cld

  # **** Switch to IA32 Protected Mode
  # set PE (Protection Enable bit 0) and 
  # WP (Write Protection bit 16) bits
  # in CR0 (Control Register 0)
  mov %cr0, %eax
  or $0x00010001, %eax
  mov %eax, %cr0

  # **** Load AP GDT ****
  lgdt (AP_TRAM_START + (_AP_GDT_value - _trampolineAP))

  # Set segmet registers
  xor %ax, %ax
  mov $KERNEL_DATA_SEL, %ax
  mov %ax, %ds
  mov %ax, %ss
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  #Far Jump with code segment selector
  jmp $KERNEL_CODE_SEL, $(AP_TRAM_START + (_StartAP - _trampolineAP))

.align 32
.code32
.global _StartAP
.type _StartAP, @function
_StartAP:

  # Get ID of the AP in ebx
  mov     $1, %eax
  cpuid
  shrl    $24, %ebx

  # Calculate base address of the record
  mov     $CPU_State, %ecx
  mov     $CPU_STATE_SIZE, %eax
  mul     %ebx
  add     %eax, %ecx

  # Access stack addr
  mov     $CPU_STATE_STACK_ADDR, %eax
  add     %ecx, %eax  
  mov     (%eax), %esp

  # Access StartupState
  mov     $CPU_STATE_START_STATE_OFF, %eax
  add     %ecx, %eax  

1:
  pause
  cmpb     $CPU_STATE_WAITING_START, (%eax)
  jne      1b

  movl    $CPU_STATE_STARTED, (%eax)

  jmp $KERNEL_CODE_SEL, $AP_main

.align 16
_AP_GDT_table:
  .long 0, 0
  .long 0x0000FFFF, 0x00CF9A00    # flat code
  .long 0x0000FFFF, 0x00CF9200    # flat data
_AP_GDT_value:
  .word _AP_GDT_value - _AP_GDT_table - 1
  .long (AP_TRAM_START + (_AP_GDT_table - _trampolineAP))
  .long 0, 0