#include <libk/stdint.h>
#include <IA32/init_defs.h>
#include <IA32/pic.h>
#include <IA32/io.h>

static uint16_t IRQ_start_offset = 0;

void PIC_disable(void)
{
  outb(0x20, PIC1_COMMAND);
  outb(0x20, PIC2_COMMAND);
  outb(0xFF, PIC1_DATA);
  outb(0xFF, PIC2_DATA);
}

void PIC_SendEOI(uint32_t irq)
{

  if(IRQ_start_offset > 0){
    irq = (irq & 0x000000FF) -  IRQ_start_offset;
  }

  if(irq <= 15){
    if(irq >= 8)
      outb(PIC_EOI, PIC2_COMMAND);
   
    outb(PIC_EOI, PIC1_COMMAND);
  }
}

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default 
arguments:
  offset1 - vector offset for master PIC
    vectors on the master become offset1..offset1+7
  offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(uint16_t offset1, uint16_t offset2)
{
  unsigned char a1, a2;

  IRQ_start_offset = offset1;
  
  a1 = inb(PIC1_DATA);                        // save masks
  a2 = inb(PIC2_DATA);
 
  outb(ICW1_INIT | ICW1_ICW4, PIC1_COMMAND);  // starts the initialization sequence (in cascade mode)
  io_wait();
  outb(ICW1_INIT | ICW1_ICW4, PIC2_COMMAND);
  io_wait();
  outb(offset1, PIC1_DATA);                 // ICW2: Master PIC vector offset
  io_wait();
  outb(offset2, PIC2_DATA);                 // ICW2: Slave PIC vector offset
  io_wait();
  outb(4, PIC1_DATA);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  io_wait();
  outb(2, PIC2_DATA);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
  io_wait();
 
  outb(ICW4_8086, PIC1_DATA);
  io_wait();
  outb(ICW4_8086, PIC2_DATA);
  io_wait();
 
  outb(a1, PIC1_DATA);   // restore saved masks.
  outb(a2, PIC2_DATA);
}

void PIC_IRQ_set_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(value, port);        
}


void PIC_IRQ_clear_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(value, port);        
}

/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(ocw3, PIC1_COMMAND);
    outb(ocw3, PIC2_COMMAND);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}
 
/* Returns the combined value of the cascaded PICs irq request register */
uint16_t PIC_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}
 
/* Returns the combined value of the cascaded PICs in-service register */
uint16_t PIC_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}


void PIC_EnableIRQ(uint32_t irq_n) {
  PIC_IRQ_clear_mask(((uint8_t)irq_n) - HW_INT_START);
}

void PIC_DisableIRQ(uint32_t irq_n) {
  PIC_IRQ_set_mask(((uint8_t)irq_n) - HW_INT_START);
}

