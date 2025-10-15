#ifndef _IA32_PIT_H_
#define _IA32_PIT_H_

#include <libk/stdint.h>

/**
 * PIT Channels
 */
#define PIT_DATA_CH0  (0x40)
#define PIT_DATA_CH1  (0x41)
#define PIT_DATA_CH2  (0x42)

/* PIT Time Constants */
#define PIT_CLOCK     (1193181) /* Hz */

/* PIT Modes */
#define PIT_INT_TERM_COUNT      (0x00)
#define PIT_HW_RETRIG_ONE_SHOT  (0x02)
#define PIT_RATE_GEN            (0x04)
#define PIT_SQUARE_WAVE_GEN     (0x05)
#define PIT_SW_TRIG_STROBE      (0x08)
#define PIT_HW_TRIG_STROBE      (0x0A)

void PIT_set_mode(uint8_t mode, uint8_t ch);
uint32_t PIT_get_count(uint8_t ch);
void PIT_set_count(uint16_t count, uint8_t ch);

#endif

