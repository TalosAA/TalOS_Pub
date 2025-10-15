#include <hal/interrupts.h>
#include <IA32/init_defs.h>
#include <IA32/sysinit.h>
#include <IA32/pic.h>
#include <IA32/apic.h>
#include <IA32/utils.h>

extern SysConf_Info_t SysConf_Info;
extern void* isr_table[INT_NUM];
extern uint32_t* IOAPIC_Addr;

uint8_t ISR_Install(uint16_t num, void* isrAddress){
  
  if(num > INT_NUM)
    return INT_ERR_OUT_OF_RANGE;

  _cli();
  isr_table[num] = isrAddress;
  _sti();

  return INT_OK;
}

uint8_t ISR_Uninstall(uint16_t num){

  if(num > INT_NUM)
    return INT_ERR_OUT_OF_RANGE;

  _cli();
  isr_table[num] = NULL;
  _sti();

  return INT_OK;
}

uint32_t GetNumberOfIntr(void) {
  return INT_NUM;
}

void EnableIRQ(uint8_t dest_cpu, uint32_t irq_n) {
  uint8_t deliveryMode = DELIVERY_MODE_FIXED;

  //TODO: verificare che l'irq sia compresa nel range di quelle
  //      gestite dagli IOAPIC.
  if(irq_n >= HW_INT_START && irq_n != SPURIOUS_INT_VECTOR) {
    if(SysConf_Info.ACPI_Init == TRUE) {

      /* If the Interrupt is registered as NMI */
      // TODO gestione NMI
      // if(SysConf_Info.InterruptSource[IRQ(irq_n)].isNMI == INT_INFO_IS_NMI) {
      //   deliveryMode = DELIVERY_MODE_NMI;
      //   SysConf_Info.InterruptSource[IRQ(irq_n)].TriggerMode = TRIG_MODE_EDGE;
      // }

      IOAPIC_IRQ_Setup(IOAPIC_Addr,
                       dest_cpu,
                       SysConf_Info.InterruptSource[IRQ(irq_n)].IntVector,
                       SysConf_Info.InterruptSource[IRQ(irq_n)].GlobalSysInt, 
                       SysConf_Info.InterruptSource[IRQ(irq_n)].TriggerMode, 
                       SysConf_Info.InterruptSource[IRQ(irq_n)].Polarity,
                       DEST_MODE_PHY,
                       deliveryMode);

      IOAPIC_Unmask_IRQ(IOAPIC_Addr,
                        SysConf_Info.InterruptSource[IRQ(irq_n)].GlobalSysInt);
    } else {
      PIC_EnableIRQ(irq_n);
    }
  }
}

void DisableIRQ(uint32_t irq_n) {
  if(irq_n >= HW_INT_START && irq_n != SPURIOUS_INT_VECTOR) {
    if(SysConf_Info.ACPI_Init == TRUE) {
      IOAPIC_Mask_IRQ(IOAPIC_Addr, irq_n);
    } else {
      PIC_DisableIRQ(irq_n);
    }
  }
}

void SoftwareInterrupt(void) {
  INT3();
}

void DisableInterrupts(void) {
  _cli();
}

void EnableInterrupts(void) {
  _sti();
}