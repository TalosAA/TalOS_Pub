#ifndef _IA32_EXC_H_
#define _IA32_EXC_H_

#include <libk/stdint.h>

/* Exceptions Codes Mapping */
#define EX_START                    (0)
#define EX_END                      (31)
#define EX_DIVIDE_BY_ZERO           (0x00)
#define EX_DEBUG                    (0x01)
#define EX_NON_MASK_INT             (0x02)
#define EX_BREAKPOINT               (0x03)
#define EX_OVERFLOW                 (0x04)
#define EX_BOUND_RANGE_EXCEEDED     (0x05)
#define EX_INVALID_OPCODE           (0x06)
#define EX_DEVICE_NOT_AVAIL         (0x07)
#define EX_DOUBLE_FAULT             (0x08)
#define EX_RESERVED_9               (0x09)
#define EX_INVALID_TSS              (0x0a)
#define EX_SEGMENT_NOT_PRESENT      (0x0b)
#define EX_STACK_SEGMENT_FAULT      (0x0c)
#define EX_GENERAL_PROTECTION_FAULT (0x0d)
#define EX_PAGE_FAULT               (0x0e)
#define EX_RESERVED_15              (0x0f)
#define EX_X87_FLOAT_POINT_EXCEPT   (0x10)
#define EX_ALIGNMENT_CHECK          (0x11)
#define EX_MACHINE_CHECK            (0x12)
#define EX_SIMD_FLOAT_POINT_EXCEPT  (0x13)
#define EX_VIRTUALIZATION_EXCEPT    (0x14)
#define EX_CONTROL_PROTECTION       (0x15)
#define EX_RESERVED_22              (0x16)
#define EX_RESERVED_23              (0x17)
#define EX_RESERVED_24              (0x18)
#define EX_RESERVED_25              (0x19)
#define EX_RESERVED_26              (0x1a)
#define EX_RESERVED_27              (0x1b)
#define EX_RESERVED_28              (0x1c)
#define EX_VMM_COMM                 (0x1d)
#define EX_SECURITY_EXCEPTION       (0x1e)
#define EX_RESERVED_31              (0x1f)

/* Functions to manage exceptions */
void ex_DivideByZero();
void ex_PageFault(uint32_t id,
                  uint32_t err,
                  uint32_t eip,
                  uint32_t cs,
                  uint32_t eflags);
void ex_DoubleFault(uint32_t id,
                  uint32_t err,
                  uint32_t eip,
                  uint32_t cs,
                  uint32_t eflags);
void ex_FaultReset();

#endif
