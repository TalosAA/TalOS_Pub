#ifndef _HAL_INTERRUPTS_H_
#define _HAL_INTERRUPTS_H_

#include <libk/stddef.h>
#include <libk/stdint.h>

/* Install Errors */
#define INT_OK                (0)
#define INT_ERR_OUT_OF_RANGE  (1)
#define INT_ERR               (2)

extern void EnableIRQ(uint8_t dest_cpu, uint32_t irq_n);
extern void DisableIRQ(uint32_t irq_n);
extern void DisableInterrupts(void);
extern void EnableInterrupts(void);
extern void SoftwareInterrupt(void);
extern uint32_t GetNumberOfIntr(void);
extern uint8_t ISR_Install(uint16_t num,
                           void* isrAddress);
extern uint8_t ISR_Uninstall(uint16_t num);


#endif