#ifndef _IA32_PIC_H_
#define _IA32_PIC_H_

#include <stdint.h>
#include <IA32/io.h>

/* Basic PIC definitions */
#define PIC1            0x20    /* IO base address for master PIC */
#define PIC2            0xA0    /* IO base address for slave PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)
#define PIC_READ_IRR    0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR    0x0b    /* OCW3 irq service next CMD read */

/* Config Definitions */
#define PIC1_START_IRQ(HW_INT_STR)   (HW_INT_STR)
#define PIC2_START_IRQ(HW_INT_STR)   (PIC1_START_IRQ(HW_INT_STR) + 8)

/* PIC Commands */
#define PIC_EOI         0x20    /* End-of-interrupt command code */

#define ICW1_ICW4       0x01    /* ICW4 (not) needed */
#define ICW1_SINGLE     0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08    /* Level triggered (edge) mode */
#define ICW1_INIT       0x10    /* Initialization - required! */
 
#define ICW4_8086       0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C    /* Buffered mode/master */
#define ICW4_SFNM       0x10    /* Special fully nested (not) */

/* PIC Functions */
void PIC_disable(void);
void PIC_SendEOI(uint32_t irq);
void PIC_remap(uint16_t offset1, uint16_t offset2);
void PIC_IRQ_set_mask(unsigned char IRQline);
void PIC_IRQ_clear_mask(unsigned char IRQline);
uint16_t PIC_get_irr(void);
uint16_t PIC_get_isr(void);
void PIC_EnableIRQ(uint32_t irq_n);
void PIC_DisableIRQ(uint32_t irq_n);

#endif
