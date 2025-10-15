#ifndef _IA32_CPU_H_
#define _IA32_CPU_H_

#include <libk/stdint.h>
#include <libk/stddef.h>
#include <IA32/cpuvendors.h>

/**
 * CPU Feautures, returned by cpuid
 * with EAX equal to 1. DW0 info is
 * returned in ECX, where DW1 info in
 * EDX.
 */
#define CPUID_FEAT_DW0_SSE3 (1 << 0) 
#define CPUID_FEAT_DW0_PCLM (1 << 1)
#define CPUID_FEAT_DW0_DTES (1 << 2)
#define CPUID_FEAT_DW0_MONI (1 << 3)  
#define CPUID_FEAT_DW0_DS_C (1 << 4)  
#define CPUID_FEAT_DW0_VMX  (1 << 5)  
#define CPUID_FEAT_DW0_SMX  (1 << 6)  
#define CPUID_FEAT_DW0_EST  (1 << 7)  
#define CPUID_FEAT_DW0_TM2  (1 << 8)  
#define CPUID_FEAT_DW0_SSSE (1 << 9)  
#define CPUID_FEAT_DW0_CID  (1 << 10)
#define CPUID_FEAT_DW0_SDBG (1 << 11)
#define CPUID_FEAT_DW0_FMA  (1 << 12)
#define CPUID_FEAT_DW0_CX16 (1 << 13) 
#define CPUID_FEAT_DW0_XTPR (1 << 14) 
#define CPUID_FEAT_DW0_PDCM (1 << 15) 
#define CPUID_FEAT_DW0_PCID (1 << 17) 
#define CPUID_FEAT_DW0_DCA  (1 << 18) 
#define CPUID_FEAT_DW0_SSE4 (1 << 19) 
#define CPUID_FEAT_DW0_X2AP (1 << 21) 
#define CPUID_FEAT_DW0_MOVB (1 << 22) 
#define CPUID_FEAT_DW0_POPC (1 << 23) 
#define CPUID_FEAT_DW0_TSC  (1 << 24) 
#define CPUID_FEAT_DW0_AES  (1 << 25) 
#define CPUID_FEAT_DW0_XSAV (1 << 26) 
#define CPUID_FEAT_DW0_OSXS (1 << 27) 
#define CPUID_FEAT_DW0_AVX  (1 << 28)
#define CPUID_FEAT_DW0_F16C (1 << 29)
#define CPUID_FEAT_DW0_RDRA (1 << 30)
#define CPUID_FEAT_DW0_HYPE (1 << 31)
 
#define CPUID_FEAT_DW1_FPU  (1 << 0)  
#define CPUID_FEAT_DW1_VME  (1 << 1)  
#define CPUID_FEAT_DW1_DE   (1 << 2)  
#define CPUID_FEAT_DW1_PSE  (1 << 3)  
#define CPUID_FEAT_DW1_TSC  (1 << 4)  
#define CPUID_FEAT_DW1_MSR  (1 << 5)  
#define CPUID_FEAT_DW1_PAE  (1 << 6)  
#define CPUID_FEAT_DW1_MCE  (1 << 7)  
#define CPUID_FEAT_DW1_CX8  (1 << 8)  
#define CPUID_FEAT_DW1_APIC (1 << 9)  
#define CPUID_FEAT_DW1_SEP  (1 << 11) 
#define CPUID_FEAT_DW1_MTRR (1 << 12) 
#define CPUID_FEAT_DW1_PGE  (1 << 13) 
#define CPUID_FEAT_DW1_MCA  (1 << 14) 
#define CPUID_FEAT_DW1_CMOV (1 << 15) 
#define CPUID_FEAT_DW1_PAT  (1 << 16) 
#define CPUID_FEAT_DW1_PSE3 (1 << 17) 
#define CPUID_FEAT_DW1_PSN  (1 << 18) 
#define CPUID_FEAT_DW1_CLFL (1 << 19) 
#define CPUID_FEAT_DW1_DS   (1 << 21) 
#define CPUID_FEAT_DW1_ACPI (1 << 22) 
#define CPUID_FEAT_DW1_MMX  (1 << 23) 
#define CPUID_FEAT_DW1_FXSR (1 << 24) 
#define CPUID_FEAT_DW1_SSE  (1 << 25) 
#define CPUID_FEAT_DW1_SSE2 (1 << 26) 
#define CPUID_FEAT_DW1_SS   (1 << 27) 
#define CPUID_FEAT_DW1_HTT  (1 << 28) 
#define CPUID_FEAT_DW1_TM   (1 << 29) 
#define CPUID_FEAT_DW1_IA64 (1 << 30)
#define CPUID_FEAT_DW1_PBE  (1 << 31)

typedef struct {
  uint32_t dw0;
  uint32_t dw1;
} IA32_cpu_feat_t;

typedef struct {
  char VendorId[CPU_VENDOR_STR_LEN];
  IA32_cpu_feat_t Features;
  uint16_t ModelId;
  uint16_t FamilyId;
  uint8_t  SteppingId;
} IA32_cpu_info_t;

void CPU_GetInfo(IA32_cpu_info_t* info);
bool CPU_IamBSP(void);
uint32_t CPU_GetCoreID(void);

unsigned int CPU_CheckMSR(void);
void CPU_GetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi);
void CPU_SetMSR(uint32_t msr, uint32_t lo, uint32_t hi);

#endif 
