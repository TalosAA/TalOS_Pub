#include <IA32/smp.h>
#include <IA32/apic.h>
#include <IA32/utils.h>
#include <hal/systime.h>

void SendInitIPI(uint32_t cpu_id) {
  /* Clear APIC error register */
  LAPIC_write_reg(LAPIC_ERR_STATUS_REG, 0);

  /* Select AP */
  LAPIC_write_reg(LAPIC_ICR_HIGH,
                  (LAPIC_read_reg(LAPIC_ICR_HIGH) & 0x00ffffff) |
                  LAPIC_GET_ID(cpu_id));

  /* Init IPI */
  LAPIC_write_reg(LAPIC_ICR_LOW,
                  (LAPIC_read_reg(LAPIC_ICR_LOW) & 0xfff00000) |
                  (DELIVERY_MODE_INIT << DELIVERY_MODE) |
                  (TRIG_MODE_LEVEL << TRIGGER_MODE) |
                  (ICR_LEVEL_ASSERT << ICR_LEVEL));
}

void SendInitIPIDeassert(uint32_t cpu_id) {
  /* Clear APIC error register */
  LAPIC_write_reg(LAPIC_ERR_STATUS_REG, 0);

  /* Select AP */
  LAPIC_write_reg(LAPIC_ICR_HIGH,
                  (LAPIC_read_reg(LAPIC_ICR_HIGH) & 0x00ffffff) |
                  LAPIC_GET_ID(cpu_id));

  /* Deassert IPI */
  LAPIC_write_reg(LAPIC_ICR_LOW,
                  (LAPIC_read_reg(LAPIC_ICR_LOW) & 0xfff00000) |
                  (DELIVERY_MODE_INIT << DELIVERY_MODE) |
                  (TRIG_MODE_LEVEL << TRIGGER_MODE) |
                  (ICR_LEVEL_DEASSERT << ICR_LEVEL));
}

void SendStartupIPI(uint32_t cpu_id, uint32_t vector) {
  /* Clear APIC error register */
  LAPIC_write_reg(LAPIC_ERR_STATUS_REG, 0);

  /* Select AP */
  LAPIC_write_reg(LAPIC_ICR_HIGH,
                  (LAPIC_read_reg(LAPIC_ICR_HIGH) & 0x00ffffff) |
                  LAPIC_GET_ID(cpu_id));

  /* Init IPI */
  LAPIC_write_reg(LAPIC_ICR_LOW,
                  (LAPIC_read_reg(LAPIC_ICR_LOW) & 0xfff00000) |
                  (DELIVERY_MODE_STARTUP << DELIVERY_MODE) |
                  (vector & 0x0FF));
}

void SendIPI(uint32_t cpu_id, uint32_t vector) {
  /* Clear APIC error register */
  LAPIC_write_reg(LAPIC_ERR_STATUS_REG, 0);

  /* Select AP */
  LAPIC_write_reg(LAPIC_ICR_HIGH,
                  (LAPIC_read_reg(LAPIC_ICR_HIGH) & 0x00ffffff) |
                  LAPIC_GET_ID(cpu_id));

  /* IPI */
  LAPIC_write_reg(LAPIC_ICR_LOW,
                  (LAPIC_read_reg(LAPIC_ICR_LOW) & 0xfff00000) |
                  (DELIVERY_MODE_FIXED << DELIVERY_MODE) |
                  (vector & 0x0FF));
}

bool IPI_isPending(void) {
  return LAPIC_read_reg(LAPIC_ICR_LOW) & (1 << DELIV_STATUS);
}

void StartupAP(uint32_t AP_Num, uint8_t vector) {
  uint8_t i = 0;

  SendInitIPI(AP_Num);

  /* Wait end */
  while (IPI_isPending()) {
    _pause();
  }

  SendInitIPIDeassert(AP_Num);

  /* Wait end */
  while (IPI_isPending()) {
    _pause();
  }

  Wait_ms(10);

  for(i = 0; i < 2; i++) {
    SendStartupIPI(AP_Num, vector);
    Wait_us(200);
    /* Wait end */
    while (IPI_isPending()) {
      _pause();
    }
  }
}