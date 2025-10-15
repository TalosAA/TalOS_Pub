#ifndef _IA32_TABS_DEF_H_
#define _IA32_TABS_DEF_H_

/* GDT Segments Definitions */
#define GDT_SEG_NUM               (6)
#define GDT_ENTRY_SIZE            (8)
#define GDTR_SIZE                 (6)

#define KERNEL_CODE_SEL_AB        (0x9A)
#define KERNEL_DATA_SEL_AB        (0x92)

#define NULL_SEL                  (0x00)
#define KERNEL_CODE_SEL           (NULL_SEL + GDT_ENTRY_SIZE)
#define KERNEL_DATA_SEL           (KERNEL_CODE_SEL + GDT_ENTRY_SIZE)
#define USER_CODE_SEL             (KERNEL_DATA_SEL + GDT_ENTRY_SIZE)
#define USER_DATA_SEL             (USER_CODE_SEL + GDT_ENTRY_SIZE)

/* IDT Definitions */
#define INTGATE                   (0x8E)
#define TRAPGATE                  (0x8E)
#define IDT_ENTRY_SIZE            (8)
#define IDTR_SIZE                 (6)

#endif