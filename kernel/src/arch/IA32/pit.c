#include <libk/stdint.h>
#include <IA32/pit.h>
#include <IA32/io.h>
#include <IA32/utils.h>

/**
 * PIT Ports
 */
#define PIT_CMD_PORT            (0x43)

void PIT_set_mode(uint8_t mode, uint8_t ch) {
  /* Set the mode */
  outb(((ch << 10) & 0xFF) | mode, PIT_CMD_PORT);
}

uint32_t PIT_get_count(uint8_t ch){
  uint32_t count;

  /* al = channel in bits 6 and 7, remaining bits clear */
  outb(ch, PIT_CMD_PORT);
 
  count = inb(ch);                   /* Low byte */
  count |= (inb(ch) << 8) & 0xFF;    /* High byte */
 
  return count;
  
}

void PIT_set_count(uint16_t count, uint8_t ch) {
 
  /* set count */
  outb(count&0xFF, ch);          /* Low byte */
  outb((count&0xFF00)>>8, ch);   /* High byte */
  
	return;
}




