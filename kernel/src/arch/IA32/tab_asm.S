#include <IA32/tab_defs.h>
#include <IA32/init_defs.h>

.section .text
.global Load_TSS
.type Load_TSS, @function
Load_TSS:
  mov $0x28, %ax
  ltr %ax
  ret

.global Load_GDT
.type Load_GDT, @function
Load_GDT:
  push %ebp
  mov  %esp, %ebp

  mov 8(%ebp), %eax
  lgdt (%eax)

  # Set segmet registers
  xor %ax, %ax
  mov $KERNEL_DATA_SEL, %ax
  mov %ax, %ds
  mov %ax, %ss
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  leave

  # Far Jump with code segment selector
  jmp $KERNEL_CODE_SEL,$1f
1:
  ret

.global Load_IDT
.type Load_IDT, @function
Load_IDT:
  push %ebp
  mov  %esp, %ebp
  mov 8(%ebp), %eax
  lidt (%eax)
  leave
  ret

.type nopIsr, @function
nopIsr:
  ret

# **** Table of ISR handlers ****
# The following array of long contains
# the addresses of the ISR to be called
# by a generic isrID handler.
.align 16
.macro isr_tab_t id
  isr_tab_\id:
    .long nopIsr
.endm

.section .data
.global isr_table
isr_table:
  .set i,0
  .rept INT_NUM
    .altmacro
    isr_tab_t %i
    .set i, i+1
  .endr

.align 16
.macro isr_addr_list_t id
    .long isr\id
.endm

.section .data
.global isr_addr_list
isr_addr_list:
  .set i,0
  .rept INT_NUM
    .altmacro
    isr_addr_list_t %i
    .set i, i+1
  .endr

# **** ISR handlers definition ****
# The following macro defines a generic
# ISR, it calls the service routine pointed
# in the isr_table and sends the EOI.
.macro IntHandDef id
  .global isr\id
  .type isr\id, @function
    isr\id:
      .if \id == SPURIOUS_INT_VECTOR
        iret
      .else
        cli
        push    %ebp
        mov     %esp, %ebp
        pushal
        .if \id ==  8 || \id == 10 || \id == 11 || \id == 12 || \id == 13 ||\
            \id == 14 || \id == 17 || \id == 21 || \id == 29 || \id == 30
          # Exceptions pushing Error Code:
          # ID  Description
          #  8  Double Fault
          # 10  Invalid TSS
          # 11  Segment Not Present
          # 12  Stack-Segment Fault
          # 13  General Protection Fault
          # 14  Page Fault
          # 17  Alignment Check
          # 21  Control Protection Exception
          # 29  VMM Communication Exception
          # 30  Security Exception
          pushl   24(%ebp)     # SS
          pushl   20(%ebp)     # ESP
          pushl   16(%ebp)     # EFLAGS
          pushl   12(%ebp)     # CS
          pushl   8(%ebp)      # EIP
          pushl   4(%ebp)      # Error Code
          mov     $28, %ebx
        .else
          pushl   20(%ebp)     # SS
          pushl   16(%ebp)     # ESP
          pushl   12(%ebp)     # EFLAGS
          pushl   8(%ebp)      # CS
          pushl   4(%ebp)      # EIP
          mov     $24, %ebx
        .endif
        pushl   $\id         # Interrupt Id
        call    *isr_tab_\id
        add     %ebx,%esp
        #  EOI is required for all the interrupts, except:
        #  NMI, SMI, INIT, ExtINT, the start-up,
        #  and INIT-Deassert delivery mode
        #  (Intel man. vol 3 11.8.5)
        .if \id != 2 #  NMI
          pushl   $\id   
          call    *SendEOI_handler
          add     $4,%esp
        .endif
        popal
        leave
        sti
        iret
      .endif
.endm

.section .text
# isr_handlers contains all the ISR definitions
.align 16
isr_handlers:
.set i,0
.rept INT_NUM
  .altmacro
  IntHandDef %i
  .set i, i+1
.endr