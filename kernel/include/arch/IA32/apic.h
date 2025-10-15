#ifndef _APIC_H_
#define _APIC_H_

#include <libk/stdint.h>
#include <libk/stddef.h>

/* Number of supported IOAPICs*/
#define MAX_SUPP_IOAPIC_NUM     (0x10)

#define APIC_BASE_MSR           (0x1B)
#define APIC_BASE_MSR_BSP       (0x100)
#define APIC_BASE_MSR_ENABLE    (0x800)

#define APIC_ENABLE_BIT         (0x100)

/* LAPIC Registers Addresses offesets */
#define LAPIC_ID_REG            (0x020)
#define LAPIC_VERSION_REG       (0x030)
#define LAPIC_TPR_REG           (0x080)
#define LAPIC_APR_REG           (0x090)
#define LAPIC_PPR_REG           (0x0A0)
#define LAPIC_EOI_REG           (0x0B0)
#define LAPIC_RRD_REG           (0x0C0)
#define LAPIC_LOGIDEST_REG      (0x0D0)
#define LAPIC_DESTFORMAT_REG    (0x0E0)
#define LAPIC_SPURIOUS_INT_REG  (0x0F0)
/* ... */
/* ISR, TMR and IRR registers (ReadOnly) */
/* ... */
#define LAPIC_ERR_STATUS_REG    (0x280)
#define LAPIC_LVT_CMCI          (0x2F0)
#define LAPIC_ICR_LOW           (0x300)
#define LAPIC_ICR_HIGH          (0x310)
#define LAPIC_LVT_TIMER         (0x320)
#define LAPIC_LVT_THERMAL_SENS  (0x330)
#define LAPIC_PERF_MON_COUNT    (0x340)
#define LAPIC_LVT_LINT0         (0x350)
#define LAPIC_LVT_LINT1         (0x360)
#define LAPIC_LVT_ERR           (0x370)
#define LAPIC_INIT_COUNT        (0x380)
#define LAPIC_CURR_COUNT        (0x390)
#define LAPIC_TIMER_DIV_CONF    (0x3E0)

/* IOAPIC Definitions */
#define IOAPIC_IOREGSEL         (0x00)
#define IOAPIC_IOWIN            (0x10)

/* IOAPIC Registers */
#define IOAPIC_ID_REG           (0x00)
#define IOAPIC_VERSION_REG      (0x01)

/* Help Macro */
#define IOAPIC_GET_ID(id)       ((0x03 & (id)) << 24)
#define LAPIC_GET_ID(id)        ((id) << 24)

#define IOPIC_LOWER_ENTRY       (0x10) 
#define IOPIC_UPPER_ENTRY       (0x11)

#define IOAPIC_ENTRY_LOWER_ADDR(line)  (IOPIC_LOWER_ENTRY + ((line) * 2))
#define IOAPIC_ENTRY_UPPER_ADDR(line)  (IOPIC_UPPER_ENTRY + ((line) * 2))

/* Delivery modes */
#define DELIVERY_MODE           (0x08)
#define DELIVERY_MODE_FIXED     (0x00)
#define DELIVERY_MODE_LOWPRI    (0x01)
#define DELIVERY_MODE_SMI       (0x02)
#define DELIVERY_MODE_NMI       (0x04)
#define DELIVERY_MODE_INIT      (0x05)
#define DELIVERY_MODE_STARTUP   (0x06)
/* Destionation Mode */
#define DESTINATION_MODE        (0x0B)
#define DEST_MODE_PHY           (0x00)
#define DEST_MODE_LOGI          (0x01)
#define DEST_MODE_INTPOL        (0x0D)
#define DEST_MODE_INTPOL_HACT   (0x00)
#define DEST_MODE_INTPOL_LACT   (0x01)
/* Trigger Mode */
#define TRIGGER_MODE            (0x0F)
#define TRIG_MODE_LEVEL         (0x01)
#define TRIG_MODE_EDGE          (0x00)
/* Level */
#define ICR_LEVEL               (0x0E)
#define ICR_LEVEL_DEASSERT      (0x00)
#define ICR_LEVEL_ASSERT        (0x01)
/* Interrupt Mask */
#define INT_MASK                (0x10)
#define INT_MASKED              (0x01)
#define INT_CLEAR               (0x00)
/* Delivery Satus */
#define DELIV_STATUS            (0x0C)
/* Remote IIR */
#define REMOTE_IRR              (0x0E)
/* Destination Field */
#define DEST_FIELD              (0x18)
#define DEST_FIELD_MASK         (0xFF000000)
#define DEST_FIELD_NBIT         (0x0F)

/* IPI */
#define IPI_DEST_SHORTHAND      (0x12)
#define IPI_DEST_SHORT_NO       (0x00)
#define IPI_DEST_SELF           (0x01)
#define IPI_DEST_ALL_SELF       (0x02)
#define IPI_DEST_ALL_EX_SELF    (0x03)

/* LAPIC Timer Definitions */
#define LAPIC_TIM_DIV_2         (0x00)
#define LAPIC_TIM_DIV_4         (0x01)
#define LAPIC_TIM_DIV_8         (0x02)
#define LAPIC_TIM_DIV_16        (0x03)
#define LAPIC_TIM_DIV_32        (0x08)
#define LAPIC_TIM_DIV_64        (0x09)
#define LAPIC_TIM_DIV_128       (0x0A)
#define LAPIC_TIM_DIV_1         (0x0B)

#define LAPIC_TIM_MODE_ONESHOT  (0x00)
#define LAPIC_TIM_MODE_PERIODIC (0x01)
#define LAPIC_TIM_MODE_TSCDEADL (0x02)

#define LAPIC_TIM_LVT_DISABLE   (0x10000)

typedef struct {
  uint32_t vector       : 8;
  uint32_t delvMode     : 3;
  uint32_t destMode     : 1;
  uint32_t delvStatus   : 1;
  uint32_t pinPolarity  : 1;
  uint32_t remoteIRR    : 1;
  uint32_t triggerMode  : 1;
  uint32_t mask         : 1;
  uint32_t reserved1    : 15;
  uint32_t reserved2    : 24;
  uint32_t destination  : 8;
} __is_packed ioapic_redirect_entry_t;

/* Utility Macros */
#define IOAPIC_GET_VERSION(IOAPIC_VER_REG)\
        BIT_FIELD_32((IOAPIC_VER_REG), 0, 8)
#define IOAPIC_GET_IRQ_NUM(IOAPIC_VER_REG)\
        BIT_FIELD_32((IOAPIC_VER_REG), 16, 8)

/* Local APIC Functionalities */
void LAPIC_init_vir_addr(uintptr_t addr);
void LAPIC_set_phy_base_addr(uintptr_t addr);
uintptr_t LAPIC_get_phy_base_addr(void); 
void LAPIC_enable(uint16_t spurious_int_vect);
uint32_t LAPIC_read_reg(uint32_t reg);
void LAPIC_write_reg(uint32_t reg, uint32_t value);
void LAPIC_Send_EOI(__is_unused uint32_t irq);
void LAPIC_send_IPI(uint8_t vector,
                    uint8_t deliveryMode,
                    uint8_t destMode,
                    uint8_t level,
                    uint8_t trigger,
                    uint8_t destShort,
                    uint8_t dest);
bool LAPIC_IPI_sent(void);

/* Local APIC timer functions */
void LAPIC_TIM_SetClockDivisor(uint8_t divisor);
void LAPIC_TIM_SetInitCount(uint32_t count);
uint32_t LAPIC_TIM_GetCurrentCount(void);
void LAPIC_TIM_StopTimer(void);
void LAPIC_TIM_SetMode(uint8_t mode);

/* I/O APIC Functionalities */
void IOAPIC_enable(uint32_t* IOAPIC_ptr, uint8_t IOAPIC_Id);
uint32_t IOAPIC_read_reg(uint32_t* IOAPIC_ptr, uint32_t reg);
void IOAPIC_write_reg(uint32_t* IOAPIC_ptr, uint32_t reg,
                      uint32_t value);
void IOAPIC_Entry_Read(uint32_t* IOAPIC_ptr, uint32_t line,
                       ioapic_redirect_entry_t* entry);
void IOAPIC_Entry_Write(uint32_t* IOAPIC_ptr, uint32_t line,
                        ioapic_redirect_entry_t* entry);
void IOAPIC_Unmask_IRQ(uint32_t* IOAPIC_ptr, uint32_t irq);
void IOAPIC_Mask_IRQ(uint32_t* IOAPIC_ptr, uint32_t irq);
void IOAPIC_IRQ_Setup(uint32_t* IOAPIC_ptr,
                      uint8_t dest,
                      uint16_t IntVector,
                      uint32_t line, 
                      uint16_t trigger, 
                      uint16_t polarity,
                      uint8_t destMode,
                      uint8_t deliveryMode);
void IOAPIC_IRQ_Change_Dest(uint32_t* IOAPIC_ptr, uint32_t dest,
                            uint32_t line);

#endif
