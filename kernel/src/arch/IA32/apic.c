#include <libk/stddef.h>
#include <libk/stdint.h>
#include <libk/bitset.h>
#include <libk/string.h>
#include <IA32/apic.h>
#include <IA32/cpu.h>
#include <IA32/io.h>
 
static volatile uint32_t* LAPIC_VirtualAddr = NULL;

static inline uintptr_t LAPIC_GetAddr(void);

void LAPIC_init_vir_addr(uintptr_t addr) {
   LAPIC_VirtualAddr = (uint32_t*)addr;
}
 
/* Set the physical address for local APIC registers */
void LAPIC_set_phy_base_addr(uintptr_t apic) {
   uint32_t edx = 0;
   uint32_t eax = (apic & 0xfffff0000) | APIC_BASE_MSR_ENABLE;
   CPU_SetMSR(APIC_BASE_MSR, eax, edx);
}

/**
 * Get the physical address of the APIC registers page
 * make sure you map it to virtual memory ;)
 */
uintptr_t LAPIC_get_phy_base_addr(void) {
   uint32_t eax, edx;
   CPU_GetMSR(APIC_BASE_MSR, &eax, &edx);
   return (eax & 0xfffff000);
}

uint32_t LAPIC_read_reg(uint32_t reg)
{
	return *((volatile uint32_t *)(LAPIC_GetAddr() + reg));
}

void LAPIC_write_reg(uint32_t reg, uint32_t value)
{
	*((volatile uint32_t *)(LAPIC_GetAddr() + reg)) = value;
}

void LAPIC_enable(uint16_t spurious_int_vect) {

   uint32_t spurious_reg_val = 0;

   spurious_reg_val = (spurious_int_vect & 0x0FF) | APIC_ENABLE_BIT;
 
   /* Set the Spurious Interrupt Vector Register bit 8
       to start receiving interrupts */
   LAPIC_write_reg(LAPIC_SPURIOUS_INT_REG, spurious_reg_val);

   /* Clear EOI register */
   LAPIC_write_reg(LAPIC_EOI_REG, 0);
}

static inline uintptr_t LAPIC_GetAddr(void) {
   if(LAPIC_VirtualAddr != NULL) {
      return (uintptr_t)LAPIC_VirtualAddr;
   } else {
      return LAPIC_get_phy_base_addr();
   }
}

void LAPIC_Send_EOI(__is_unused uint32_t irq) {
   LAPIC_write_reg(LAPIC_EOI_REG, 0);
}

void LAPIC_send_IPI(uint8_t vector,
                    uint8_t deliveryMode,
                    uint8_t destMode,
                    uint8_t level,
                    uint8_t trigger,
                    uint8_t destShort,
                    uint8_t dest) {
   uint32_t toWriteLow = 0;
   uint32_t toWriteHigh = 0;
   toWriteLow |= vector & 0x0FF;
   toWriteLow |= (deliveryMode & 0x03) << 8;
   toWriteLow |= (destMode & 0x01) << 11;
   toWriteLow |= (level & 0x01) << 14;
   toWriteLow |= (trigger & 0x01) << 15;
   toWriteLow |= (destShort & 0x03) << 18;
   toWriteHigh |= (dest & 0x0FF) << 24;
   LAPIC_write_reg(LAPIC_ICR_LOW, toWriteLow);
   LAPIC_write_reg(LAPIC_ICR_HIGH, toWriteHigh);
}

bool LAPIC_IPI_sent(void) {
   uint32_t toRead = 0;
   toRead = LAPIC_read_reg(LAPIC_ICR_LOW);
   if(BIT_FIELD_32(toRead, 12, 1) == 0) {
      return true;
   }
   return false;
}

void LAPIC_TIM_SetClockDivisor(uint8_t divisor) {
   LAPIC_write_reg(LAPIC_TIMER_DIV_CONF, BIT_FIELD_32(divisor, 0, 3));
}

void LAPIC_TIM_SetInitCount(uint32_t count) {
   LAPIC_write_reg(LAPIC_INIT_COUNT, count);
}

uint32_t LAPIC_TIM_GetCurrentCount(void) {
   return LAPIC_read_reg(LAPIC_CURR_COUNT);
}

void LAPIC_TIM_StopTimer(void) {
   LAPIC_write_reg(LAPIC_INIT_COUNT, 0);
}

void LAPIC_TIM_SetMode(uint8_t mode) {
   LAPIC_write_reg(LAPIC_LVT_TIMER, BIT_FIELD_32(mode, 17, 2));
}

void IOAPIC_enable(uint32_t* IOAPIC_ptr, uint8_t IOAPIC_Id) {
  uint32_t IOAPIC_VerReg = 0; 
  uint8_t i = 0;

  /* Set IOAPIC Id */
  IOAPIC_write_reg(IOAPIC_ptr, IOAPIC_ID_REG, IOAPIC_GET_ID(IOAPIC_Id));

  IOAPIC_VerReg = IOAPIC_read_reg(IOAPIC_ptr, IOAPIC_VERSION_REG);
  
  /* Disable all interrupts by default */
  for (i = 0; i < IOAPIC_GET_IRQ_NUM(IOAPIC_VerReg); i++) {
    IOAPIC_Mask_IRQ(IOAPIC_ptr, i); 
  }
}

uint32_t IOAPIC_read_reg(uint32_t* IOAPIC_ptr, uint32_t reg) {
   /* Write the address of the register in IOREGSEL */
   *((volatile uint32_t*)IOAPIC_ptr) = reg;
   /* Read the value in IOWIN */
   return\
   *((volatile uint32_t*)((uint32_t)IOAPIC_ptr + IOAPIC_IOWIN));
}

void IOAPIC_write_reg(uint32_t* IOAPIC_ptr, uint32_t reg, uint32_t value) {
   /* Write the address of the register in IOREGSEL */
   *((volatile uint32_t*)IOAPIC_ptr) = reg;
   /* Write the value in IOWIN */
   *((volatile uint32_t*)((uint32_t)IOAPIC_ptr + IOAPIC_IOWIN)) =\
   value;
}

void IOAPIC_Entry_Read(uint32_t* IOAPIC_ptr, uint32_t line, ioapic_redirect_entry_t* entry)
{
  ((uint32_t*)entry)[0] = IOAPIC_read_reg(IOAPIC_ptr, IOAPIC_ENTRY_LOWER_ADDR(line));
  ((uint32_t*)entry)[1] = IOAPIC_read_reg(IOAPIC_ptr, IOAPIC_ENTRY_UPPER_ADDR(line));
}

void IOAPIC_Entry_Write(uint32_t* IOAPIC_ptr, uint32_t line, ioapic_redirect_entry_t* entry)
{
  IOAPIC_write_reg(IOAPIC_ptr, IOAPIC_ENTRY_LOWER_ADDR(line), ((uint32_t*)entry)[0]);
  IOAPIC_write_reg(IOAPIC_ptr, IOAPIC_ENTRY_UPPER_ADDR(line), ((uint32_t*)entry)[1]);
}

void IOAPIC_Unmask_IRQ(uint32_t* IOAPIC_ptr, uint32_t irq)
{
  ioapic_redirect_entry_t entry;
  IOAPIC_Entry_Read(IOAPIC_ptr, irq, &entry);
  entry.mask = 0;
  IOAPIC_Entry_Write(IOAPIC_ptr, irq, &entry);
}

void IOAPIC_Mask_IRQ(uint32_t* IOAPIC_ptr, uint32_t irq)
{
  ioapic_redirect_entry_t entry;
  IOAPIC_Entry_Read(IOAPIC_ptr, irq, &entry);
  entry.mask = 1;
  IOAPIC_Entry_Write(IOAPIC_ptr, irq, &entry);
}

void IOAPIC_IRQ_Setup(uint32_t* IOAPIC_ptr,
                      uint8_t dest,
                      uint16_t IntVector,
                      uint32_t line, 
                      uint16_t trigger, 
                      uint16_t polarity,
                      uint8_t destMode,
                      uint8_t deliveryMode)
{
  ioapic_redirect_entry_t entry;
  ((uint32_t*)&entry)[0] = 0;
  ((uint32_t*)&entry)[1] = 0;
  entry.destination = dest & 0x0FF;
  entry.mask = INT_MASKED;
  entry.triggerMode = trigger & 0x01;
  entry.pinPolarity = polarity & 0x01;
  entry.destMode = destMode;
  entry.delvMode = deliveryMode;
  entry.vector = IntVector & 0x0FF;
  IOAPIC_Entry_Write(IOAPIC_ptr, line, &entry);
}

void IOAPIC_IRQ_Change_Dest(uint32_t* IOAPIC_ptr,
                            uint32_t dest,
                            uint32_t line)
{
  ioapic_redirect_entry_t entry;
  IOAPIC_Entry_Read(IOAPIC_ptr, line, &entry);
  entry.destination = dest & 0x0FF;
  IOAPIC_Entry_Write(IOAPIC_ptr, line, &entry);
}