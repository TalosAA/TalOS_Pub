#ifndef __IA32_UTILS_H_
#define __IA32_UTILS_H_

#include <libk/stdint.h>

#define _cli() { __asm__ __volatile__ ("cli"); }
#define _sti() { __asm__ __volatile__("sti"); }
#define _nop() { __asm__ __volatile__("nop"); }
#define _lock() { __asm__ __volatile__("lock"); }
#define _hlt() { __asm__ __volatile__("hlt"); }
#define _pause() { __asm__ __volatile__("pause"); }
#define _fault_reset() { __asm__ __volatile__("JMP $0xFFFF, $0"); }
#define _clear_stack_pointer() { __asm__ __volatile__("movl $0, %esp"); }

#define TRAP(__num) __asm__ __volatile__ ("INT $" #__num)

#define INT3() __asm__ __volatile__ ("INT3")

#define BCD2DEC(bcd) (((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F))

#define UINT32(n) ((uint32_t)n)

#define TO_CONST_STR(__STR) #__STR

#endif
