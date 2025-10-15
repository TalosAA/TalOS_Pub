#include <IA32/memdefs.h>
#include <IA32/init_defs.h>

# Multiboot Header Definitions
.set ALIGN,    1<<0               # align loaded modules on page boundaries
.set MEMINFO,  1<<1               # provide memory map
.set FLAGS,    ALIGN | MEMINFO    # this is the Multiboot 'flag' field
.set HEADER_MAGIC,    0x1BADB002  # 'magic number' lets bootloader find the header
.set CHECKSUM, -(HEADER_MAGIC + FLAGS)   # checksum of above, to prove we are multiboot

.set MULTIBOOT_BOOTLOADER_MAGIC, 0x2BADB002

.globl multiboot_info
multiboot_info:
    .long 0

# Declare a header according to multiboot standard
.section .text.boot
.align 4
.global _multiboot
_multiboot:
.long HEADER_MAGIC
.long FLAGS
.long CHECKSUM

# System Entry Pointer
.global _start
.type _start, @function
_start:

  # Disable interrupt during BOOT sequence
  cli

  # **** Switch to IA32 Protected Mode
  # set PE (Protection Enable bit 0) and
  # WP (Write Protection bit 16) bits in
  # CR0 (Control Register 0)
  mov %cr0, %ecx
  or $0x00010001, %ecx
  mov %ecx, %cr0

  # Set the stack of the kernel
  movl $(BSP_stack_top - 4), %esp

  # Push the pointer to the Multiboot information structure.
  cmp  $MULTIBOOT_BOOTLOADER_MAGIC, %eax
  jne  HANG_LOOP
  mov  %ebx, (multiboot_info)
  
  # **** Read system configuration from BIOS/UEFI ****
  call InitSystemConfig

  # **** Init Paging ****
  call InitPaging

  # **** Initialize IA32 system ****
  call InitTables

  # Call the global constructors.
  call _init

  # # Transfer control to the kernel main.
  call kernel_main

  # Hang if kernel_main unexpectedly returns.
.global SYSTEM_HANG
SYSTEM_HANG:
  cli
HANG_LOOP:
  hlt
  jmp HANG_LOOP

# Reserve a stack for the KERNEL
.section .stack
.align 16
stack_bottom:
.skip CORE_STACK_SIZE
.global BSP_stack_top
BSP_stack_top: