#ifndef _IA32_INIT_DEFS_H_
#define _IA32_INIT_DEFS_H_

#define MAX_SUPP_PROC_NUM         (0x10)

/**
 * TODO: to verify if it is required to manage bus type
 */
#define INT_INFO_BUS_ISA          (0)
#define INT_INFO_IS_NMI           (1)

#define SYS_CONF_READ_ERROR       ((errno_t)-1)
#define SYS_CONF_READ_OK          ((errno_t)0)

#define SPURIOUS_INT_VECTOR       (0xFF)

/* Interrupts definitions */
#define INT_NUM                   (256)
#define HW_INT_START              (0x20)

/* Get IRQ index */
#define IRQ(i)                    (i - HW_INT_START)

/* Standard ISA Static Defined Interrupts */
#define ISA_IRQ0_PIT              (0)
#define ISA_IRQ1_KEYBOARD         (1)
#define ISA_IRQ2_CASCADE          (2)
#define ISA_IRQ3_COM2             (3)
#define ISA_IRQ4_COM1             (4)
#define ISA_IRQ5_LPT2             (5)
#define ISA_IRQ6_FLOPPY           (6)
#define ISA_IRQ7_LTP1_SPURIOUS    (7)
#define ISA_IRQ8_RTC              (8)
#define ISA_IRQ9_FREE1            (9)
#define ISA_IRQ10_FREE2           (10)
#define ISA_IRQ11_FREE3           (11)
#define ISA_IRQ12_PS2_MOUSE       (12)
#define ISA_IRQ13_FPU             (13)
#define ISA_IRQ14_PRIM_ATA_HD     (14)
#define ISA_IRQ15_SEC_ATA_HD      (15)

/**
 * CPU State definitions.
 * This definitions are used by assembly to access
 * the struct Core_Status_t defined in sysinit.h
 * NOTE: the following size definition should be changed
 * in 64-bits mode.
 */

#define CPU_STATE_SIZE            (12)
#define CPU_STATE_ID_OFF          (0)
#define CPU_STATE_START_STATE_OFF (4)
#define CPU_STATE_STACK_ADDR      (8)

/**
 * CPU State started definitions.
 */
#define CPU_STATE_NOT_STARTED     (0)
#define CPU_STATE_WAITING_START   (1)
#define CPU_STATE_STARTED         (2)

#endif
