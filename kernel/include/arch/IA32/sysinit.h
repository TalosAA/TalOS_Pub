#ifndef _IA32_PLAT_H_
#define _IA32_PLAT_H_

#include <libk/stdint.h>
#include <libk/stddef.h>
#include <hal/platform.h>
#include <IA32/init_defs.h>
#include <IA32/tab_defs.h>
#include <IA32/acpi.h>
#include <IA32/apic.h>
#include <IA32/cpu.h>
#include <IA32/tables.h>

#define BSP_INDEX     (0)

/* Send End Of Interrupt function ptr type */
typedef void SendEOI_t(uint32_t irq);

typedef struct {
  uint8_t CoreID;
  uint32_t LAPIC_Id;
} CPU_SysConf_Info_t;

typedef struct {
  uint8_t IOAPIC_ID;
  uint32_t IOAPIC_Addr;
  uint32_t GlobalSysInterBase;
} IOAPIC_SysConf_Info_t;

typedef struct {
  uint16_t BUS; /* TODO: verify bus management */
  uint16_t IntVector;
  uint32_t GlobalSysInt;
  uint16_t Polarity : 2;
  uint16_t TriggerMode : 2;
  uint16_t Reserved : 12;
  uint16_t isNMI;
} Int_SysConf_Info_t;

typedef struct {
  IA32_cpu_info_t       CPU_Info;
  CPU_SysConf_Info_t    CPUs[MAX_SUPP_PROC_NUM];
  Int_SysConf_Info_t    InterruptSource[MAX_INT_NUM];
  IOAPIC_SysConf_Info_t IOAPICs[MAX_SUPP_IOAPIC_NUM];
  uint32_t*             LAPIC_Ptr;
  uint32_t              LAPIC_tim_freqHz;
  uint8_t               CPU_Num;
  uint8_t               IOAPIC_Num;
  uint8_t               ACPI_Init;
} SysConf_Info_t;

typedef struct __is_packed {
  uint32_t CoreID;
  uint32_t StartedState;
  void* stackTopAddress;
} Core_Status_t;

typedef struct __is_packed {
  gdt_entry_t gdt[GDT_SEG_NUM];
  tss_entry_t kernelTSS;
  sGdtr_t     sGdtr;
  idt_entry_t idt[INT_NUM];
  sIdtr_t     sIdtr;
} Core_Tables_t;

errno_t ReadSystemConfig(void);
void InitPaging(void);
void InitSystemConfig(void);
void InitSysTime(void);
void InitMicroTimer(void);
void InitTables(void);
void Set_Kernel_TSS_StackPointer(uint32_t coreID,
                                 uint32_t esp);

#endif