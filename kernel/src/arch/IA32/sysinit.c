#include <libk/stdint.h>
#include <libk/string.h>
#include <libk/stdio.h>
#include <libk/kermsg.h>
#include <libk/kheap.h>
#include <coresys/kernel.h>
#include <IA32/init_defs.h>
#include <IA32/cpu.h>
#include <IA32/pic.h>
#include <IA32/acpi.h>
#include <IA32/apic.h>
#include <IA32/pit.h>
#include <IA32/paging.h>
#include <IA32/rtc.h>
#include <IA32/sysinit.h>
#include <IA32/utils.h>
#include <IA32/cpuid.h>
#include <IA32/smp.h>
#include <IA32/tables.h>
#include <IA32/exceptions.h>
#include <IA32/multiboot.h>
#include <hal/interrupts.h>
#include <hal/mmap.h>
#include <hal/init.h>


/** 
 *  End of interrupt handler.
 *  This handler is used by a generic ISR
 *  to let the cpu know that the IRQ has
 *  been serviced.
 */
SendEOI_t* SendEOI_handler                     = NULL;

/**
 * Multi-processing information (from ACPI)
 */
SysConf_Info_t SysConf_Info                    = { 0 };

/**
 * CPUs Status
 */
uint8_t       CPU_Started_Num = 0;
Core_Status_t CPU_State[MAX_CPU_NUM];
extern void*  BSP_stack_top;
extern void   _trampolineAP(void);

/**
 * IOAPIC Virtual Address
 * TODO: Manage multi-IOAPIC
 */
uint32_t* IOAPIC_Addr                               = NULL;

/**
 *  IA32 Tables
 */
Core_Tables_t  BSP_Tables;
Core_Tables_t* AP_Tables = NULL;

/** 
 *  ISR table. This table contains the addresses of
 *  all the ISRs.
 */
extern void* isr_table[INT_NUM];
extern void* isr_addr_list[INT_NUM];

/**
 * Static Functions Prototypes
 */
static void Default_ISR(uint32_t code);
static void Set_EOI_handler(SendEOI_t* fun);
static void Init_GDTi(gdt_entry_t* gdti,
                      sGdtr_t* sGdtri,
                      tss_entry_t* tss);
static void Init_IDTi(idt_entry_t* idti,
                      sIdtr_t* sIdtri,
                      void** isr_addr_list,
                      uint32_t isr_addr_listLen);
static void Set_GDT_TSS_descriptor(gdt_entry_t* inGdt,
                                   tss_entry_t* inTSS,
                                   int32_t vector,
                                   uint16_t ss0,
                                   uint32_t esp0);

void Set_Kernel_TSS_StackPointer(uint32_t coreID, uint32_t esp) {
  if(coreID == BSP_INDEX) {
    BSP_Tables.kernelTSS.esp0 = esp;
  } else {
    AP_Tables[coreID - 1].kernelTSS.esp0 = esp;
  }
}

errno_t ReadSystemConfig(void) {
  MADT_info_t MADT_Info;
  uint16_t i = 0;
  uint16_t k = 0;
  errno_t retCode = SYS_CONF_READ_ERROR;
  RSDP_descr_t* rsdp = NULL;

  /**
   * Get CPU information
   */
  CPU_GetInfo(&SysConf_Info.CPU_Info);

  /**
   * Get system configuration info by parsing ACPI tables
  */
  
  /* Find RSDP pointer inside the BIOS memory area */
  rsdp = acpi_get_RSDP_ptr(BIOS_START_ADDR, BIOS_END_ADDR);

  if (rsdp != NULL && acpi_get_MADT_info(rsdp, &MADT_Info) == TRUE) {

    /* Populate MP Infos */
    SysConf_Info.LAPIC_Ptr = (uint32_t*)MADT_Info.LAPIC_ptr;

    for(i = 0; (i < MADT_Info.LAPICs_Num && i < MAX_SUPP_PROC_NUM); i++) {
      SysConf_Info.CPUs[i].CoreID = MADT_Info.LAPICs[i]->LAPIC_ProcId;
      SysConf_Info.CPUs[i].LAPIC_Id = MADT_Info.LAPICs[i]->LAPIC_Id;
    }
    SysConf_Info.CPU_Num = i;

    /* I/O APIC Info */
    for(i = 0; (i < MADT_Info.IOAPIC_Num && i < MAX_SUPP_IOAPIC_NUM); i++) {
      SysConf_Info.IOAPICs[i].IOAPIC_ID =\
                                    MADT_Info.IOAPICs[i]->IOAPIC_ID;
      SysConf_Info.IOAPICs[i].IOAPIC_Addr =\
                                    MADT_Info.IOAPICs[i]->IOAPIC_Addr;
      SysConf_Info.IOAPICs[i].GlobalSysInterBase =\
                                    MADT_Info.IOAPICs[i]->GlobalSysInterBase;
    }
    SysConf_Info.IOAPIC_Num = i;

    /* Initialize interrupts (Default configuration) */
    for(i = 0; i < INT_NUM; i++){
      SysConf_Info.InterruptSource[i].IntVector = HW_INT_START + i;
      SysConf_Info.InterruptSource[i].GlobalSysInt = i;
      SysConf_Info.InterruptSource[i].Polarity = 0;
      SysConf_Info.InterruptSource[i].TriggerMode = 0;
      SysConf_Info.InterruptSource[i].isNMI = 0;
    }

    /* Find interrupt source overrides */
    for(i = 0; (i < MADT_Info.IntSrcOverride_Num && i < INT_NUM); i++) {

      k = MADT_Info.IntSrcOverride[i]->IRQSource;
      SysConf_Info.InterruptSource[k].IntVector = k + HW_INT_START;
      SysConf_Info.InterruptSource[k].GlobalSysInt =\
      MADT_Info.IntSrcOverride[i]->GlobalSysInt;
      SysConf_Info.InterruptSource[k].TriggerMode =\
      (MADT_Info.IntSrcOverride[i]->Flags >> 2);
      SysConf_Info.InterruptSource[k].Polarity =\
      (MADT_Info.IntSrcOverride[i]->Flags & 0x03);
    }

    /* Find NMI */
    for(i = 0; (i < MADT_Info.NMI_Num && i < INT_NUM); i++) {
      k = 0;
      while(k < INT_NUM) {
        if(MADT_Info.NMI_Source[i]->GlobalSysInt ==
           SysConf_Info.InterruptSource[k].GlobalSysInt) {

          SysConf_Info.InterruptSource[k].isNMI = INT_INFO_IS_NMI;
          SysConf_Info.InterruptSource[k].TriggerMode =\
                                      (MADT_Info.NMI_Source[i]->Flags >> 2);
          SysConf_Info.InterruptSource[k].Polarity =\
                                      (MADT_Info.NMI_Source[i]->Flags & 0x03);                                 
          break;
        }
        k++;
      }
    }    
    retCode = SYS_CONF_READ_OK;
  }

  return retCode;
}

void InitSystemConfig(void) {
  /* Read ACPI info */
  if(ReadSystemConfig() != SYS_CONF_READ_ERROR) {
    SysConf_Info.ACPI_Init = TRUE;
  } else {
    SysConf_Info.ACPI_Init = FALSE;
  }
}

void AP_init(void) {
  uint8_t CoreId = 0;

  /* Enable paging (same of the BSP) */
  paging_enable();

  /* Initialize GDT area of the AP */
  CoreId = CPU_GetCoreID();
  Init_GDTi(AP_Tables[CoreId].gdt,
            &AP_Tables[CoreId].sGdtr,
            &AP_Tables[CoreId].kernelTSS);

  /* Load the same IDT of the BSP */
  Init_IDTi(AP_Tables[CoreId].idt,
            &AP_Tables[CoreId].sIdtr,
            isr_addr_list,
            INT_NUM);

  /* Enable LAPIC and interrupts */
  LAPIC_enable(SPURIOUS_INT_VECTOR);
  _sti();
}

void AP_main(void) {
  AP_init();
  /* Waiting for interrupt */
  while(1) {
    _hlt();
  }
}

void InitTables(void) {
  uint32_t i = 0;
  Init_GDTi(BSP_Tables.gdt, &BSP_Tables.sGdtr, &BSP_Tables.kernelTSS);
  for (i = 0; i < INT_NUM; i++) {
    isr_table[i] = Default_ISR;
  }
  Init_IDTi(BSP_Tables.idt, &BSP_Tables.sIdtr, isr_addr_list, INT_NUM);
}

void InitAPs(void) {
  if(SysConf_Info.ACPI_Init == TRUE && SysConf_Info.CPU_Num > 1) {
    uint32_t i = 0;
    uint32_t BSP_ID = CPU_GetCoreID();
    uint32_t AP_TablesSize = 0;

    /* Copy AP Trampoline code */
    memcpy((void*)AP_TRAM_START, (void*)&_trampolineAP, PAGE_SIZE);

    /* Alloc pages for the the tables of the APs */
    AP_TablesSize = SysConf_Info.CPU_Num * sizeof(Core_Tables_t);
    AP_Tables = kmalloc(AP_TablesSize);
    if(AP_Tables == NULL) {
      //TODO: panic
      while(1);
    }

    /* Start AP CPUs */
    for(i = 0; i < SysConf_Info.CPU_Num; i++) {
      if(i != BSP_ID) {
        CPU_State[i].CoreID = i;
        CPU_State[i].stackTopAddress = kmalloc(CORE_STACK_SIZE);
        if(AP_Tables == NULL) {
          //TODO: to manage
          while(1);
        }
        CPU_State[i].stackTopAddress =(void*)\
        (((uintptr_t)CPU_State[i].stackTopAddress) + CORE_STACK_SIZE - 4);
        
        StartupAP(i, AP_TRAM_START >> 12);
        CPU_State[i].StartedState = CPU_STATE_WAITING_START;
      }
      Wait_ms(10);
    }
    for(i = 0; i < SysConf_Info.CPU_Num; i++) {
      if(CPU_State[i].StartedState == CPU_STATE_STARTED) {
        CPU_Started_Num++;
      }
    }
    kermsg_info("Number of ready cores: ");
    printf("%u\n", CPU_Started_Num);
  }
}

void InitSys(void) {
  uint32_t* LAPIC_ptr;

  /* Disable interrupts */
  _cli();

  LAPIC_ptr = NULL;

  if((SysConf_Info.CPU_Info.Features.dw1 &
      CPUID_FEAT_DW1_APIC) == CPUID_FEAT_DW1_APIC &&
      SysConf_Info.ACPI_Init) {

    kermsg_info("APIC is available and will be used.\n");

    PIC_disable();

    /* Map the APIC address in the virtual memory */
    if(SysConf_Info.LAPIC_Ptr != (uint32_t*) LAPIC_get_phy_base_addr()) {
      LAPIC_set_phy_base_addr((uintptr_t)SysConf_Info.LAPIC_Ptr);
    }

    /* Map LAPIC and IOAPIC in the virtual memory of the kernel */
    LAPIC_ptr = mem_map((void*)SysConf_Info.LAPIC_Ptr,
                        (void*)SysConf_Info.LAPIC_Ptr,
                        PAGE_SIZE,
                        KMEM_RW,
                        KMEM_UNCACHEABLE,
                        NULL);
    if(LAPIC_ptr == NULL) {
      kermsg_print_fatal_error("LAPIC pagination error.");
      StopSystem();
    }

    /* Set LAPIC Virtual Address */
    LAPIC_init_vir_addr((uintptr_t)LAPIC_ptr);

    /* TODO: Multi-IOAPIC management: Map the addresses of all IOAPICS */
    /* Map IOAPIC address */
    IOAPIC_Addr = mem_map((void*)SysConf_Info.IOAPICs[0].IOAPIC_Addr,
                          (void*)SysConf_Info.IOAPICs[0].IOAPIC_Addr,
                          PAGE_SIZE,
                          KMEM_RW,
                          KMEM_UNCACHEABLE,
                          NULL);
    if(IOAPIC_Addr == NULL) {
      kermsg_print_fatal_error("I/O APIC pagination error.");
      StopSystem();
    }

    /* Reset paging to load new settings */
    paging_reset();

    /* Set EOI function */
    Set_EOI_handler(&LAPIC_Send_EOI);

    /* Enable LAPIC */
    LAPIC_enable(SPURIOUS_INT_VECTOR); 

    /* TODO: Multi-IOAPIC management: Enable all the IOAPIC detected */

    SysConf_Info.ACPI_Init = TRUE;

    kermsg_info("Number of Cores: ");
    printf("%d\n", SysConf_Info.CPU_Num);
    kermsg_info("Number of I/O APICs: ");
    printf("%d\n", SysConf_Info.IOAPIC_Num);

  } else { /* Otherwise use the PIC */
    kermsg_info("APIC is not available. The PIC will be used.\n");
    kermsg_info("Number of Cores: 1\n");

    /* Set EOI function */
    Set_EOI_handler(&PIC_SendEOI);

    /* Remap ISRs (start from 0x20) */
    PIC_remap(PIC1_START_IRQ(HW_INT_START), PIC2_START_IRQ(HW_INT_START));
 
    SysConf_Info.ACPI_Init = FALSE;
  }

  /* Initialize System clocks */
  RTC_Init();
  InitSysTime();

  /* TODO: initialize exceptions management */
  ISR_Install(EX_DOUBLE_FAULT,  ex_DoubleFault);
  ISR_Install(EX_PAGE_FAULT,  ex_PageFault);

  /* Enable Interrupts */
  _sti();

  /* Init u-sec timer if present */
  InitMicroTimer();

  /* Initialize CPUs state */
  memset(CPU_State, 0, sizeof(CPU_State));
  CPU_State[BSP_INDEX].CoreID = CPU_GetCoreID();
  CPU_State[BSP_INDEX].StartedState = CPU_STATE_STARTED;
  CPU_State[BSP_INDEX].stackTopAddress = BSP_stack_top;

  InitAPs();
}

/**
 * HAL Functions Definition
 */
void StopSystem(void) {
  _clear_stack_pointer();
  _cli();
  while(TRUE_SAFE){
    _hlt();
  }
}

void ResetSystem(void) {
  _clear_stack_pointer();
  _fault_reset();
  /* Infinite loop if the reset fails */
  while(TRUE_SAFE){
    _hlt();
  }
}

void SpinDelay(void){
  _pause();
}

void GetSysConfiguration(SysConf_t* SysConf) {
  SysConf->CoresNum = SysConf_Info.CPU_Num;
  snprintf(SysConf->CPU_InfoString,
           CPU_INFO_STRING_LEN,
           "%s - Family %d; Model %d; Stepping %d",
           SysConf_Info.CPU_Info.VendorId,
           SysConf_Info.CPU_Info.FamilyId,
           SysConf_Info.CPU_Info.ModelId,
           SysConf_Info.CPU_Info.SteppingId);
}

/**
 * Static Functions Definitions
 */
static void Default_ISR(uint32_t code) {
  printf("IRQ raised but not managed; code: %d\n", code);
}

static void Set_EOI_handler(SendEOI_t* fun) {
  SendEOI_handler = fun;
}

static void Init_GDTi(gdt_entry_t* gdti,
                      sGdtr_t* sGdtri,
                      tss_entry_t* tss) {
  /* NULL DESCRIPTOR */
  Set_GDT_descriptor(gdti, 0, 0x00000000, 0x00000000, 0x00, 0x00);
  /* KERNEL MODE CODE ENTRY */
  Set_GDT_descriptor(gdti, 1, 0xFFFFFFFF, 0x00000000, 0x9A, 0xCF);
  /* KERNEL MODE DATA ENTRY */
  Set_GDT_descriptor(gdti, 2, 0xFFFFFFFF, 0x00000000, 0x92, 0xCF);
  /* USER MODE CODE ENTRY */
  Set_GDT_descriptor(gdti, 3, 0xFFFFFFFF, 0x00000000, 0xFA, 0xCF);
  /* USER MODE DATA ENTRY */
  Set_GDT_descriptor(gdti, 4, 0xFFFFFFFF, 0x00000000, 0xF2, 0xCF);
  Set_GDT_TSS_descriptor(gdti, tss, 5, KERNEL_DATA_SEL, 0);

  sGdtri->base = (uintptr_t)&gdti[0];
  sGdtri->limit = (uint16_t)sizeof(gdt_entry_t) * GDT_SEG_NUM - 1;

  /* Load GDT */
  Load_GDT(sGdtri);
  Load_TSS();
}

static void Init_IDTi(idt_entry_t* idti,
                      sIdtr_t* sIdtri,
                      void** isr_addr_list,
                      uint32_t isr_addr_listLen) {
  register uint32_t i;

  sIdtri->base = (uintptr_t)&idti[0];
  sIdtri->limit = (uint16_t)sizeof(idt_entry_t) * INT_NUM - 1;

  for (i = 0; i < isr_addr_listLen; i++) {
    Set_IDT_descriptor(idti, i, isr_addr_list[i], INTGATE);
  }

  /* Load idt */
  Load_IDT(sIdtri);
}

static void Set_GDT_TSS_descriptor(gdt_entry_t* inGdt,
                                   tss_entry_t* inTSS,
                                   int32_t vector,
                                   uint16_t ss0,
                                   uint32_t esp0)
{
   uint32_t base = (uint32_t) inTSS;
   uint32_t limit = sizeof(tss_entry_t);

   Set_GDT_descriptor(inGdt, vector, limit, base, 0x89, 0x00);

   memset(inTSS, 0, sizeof(tss_entry_t));

   inTSS->ss0  = ss0;  /* TSS kernel stack segment. */
   inTSS->esp0 = esp0; /* TSS stack pointer. */

   /* Set the segments to be used with this TSS. These are the KERNEL
      segments with the RPL set to 0x03 (the segment can be accessed
      coming to rin 3).*/
   inTSS->cs = KERNEL_CODE_SEL;
   inTSS->ss = KERNEL_DATA_SEL;
   inTSS->ds = KERNEL_DATA_SEL;
   inTSS->es = KERNEL_DATA_SEL;
   inTSS->fs = KERNEL_DATA_SEL;
   inTSS->gs = KERNEL_DATA_SEL;
}