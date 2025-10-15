#include <libk/bitset.h>
#include <libk/string.h>
#include <IA32/cpu.h>
#include <IA32/cpuid.h>
#include <IA32/cpuvendors.h>
#include <IA32/apic.h>

#define CPUID_FLAG_MSR              (1 << 5)

#define CPUID_GETVENDORSTRING       (0)
#define CPUID_GETFEATURES           (1)
#define CPUID_GETTLB                (2)
#define CPUID_GETSERIAL             (3)
 
#define CPUID_INTELEXTENDED         (0x80000000)
#define CPUID_INTELFEATURES         (CPUID_INTELEXTENDED | 0x01)
#define CPUID_INTELBRANDSTRING      (CPUID_INTELEXTENDED | 0x02)
#define CPUID_INTELBRANDSTRINGMORE  (CPUID_INTELEXTENDED | 0x03)
#define CPUID_INTELBRANDSTRINGEND   (CPUID_INTELEXTENDED | 0x04)

uint32_t CPU_GetCoreID(void) {
  unsigned int unused = 0;
  unsigned int ebx = 0;
  __cpuid(CPUID_GETFEATURES, unused, ebx, unused, unused);
  return (ebx >> 24);
}

void CPU_GetInfo(IA32_cpu_info_t* info) {
  unsigned int eax = 0;
  unsigned int ebx = 0;
  unsigned int ecx = 0;
  unsigned int edx = 0;

  memset(info, 0, sizeof(IA32_cpu_info_t));

  /* Vendor String */
  __cpuid(CPUID_GETVENDORSTRING, eax, ebx, ecx, edx);
  *((uint32_t*)info->VendorId) = ebx;
  *((uint32_t*)(info->VendorId + 4)) = edx;
  *((uint32_t*)(info->VendorId + 8)) = ecx;
  info->VendorId[CPU_VENDOR_STR_LEN - 1] = 0;

  /* Processor Features */
  __cpuid(CPUID_GETFEATURES, eax, ebx, ecx, edx);
  info->Features.dw0 = ecx;
  info->Features.dw1 = edx;
  /* Ids parsing */
  if(BIT_FIELD_32(eax, 8, 4) == 15){
    info->FamilyId = BIT_FIELD_32(eax, 8, 4) +\
                     BIT_FIELD_32(eax, 20, 8);
  } else {
    info->FamilyId = BIT_FIELD_32(eax, 8, 4);
  }
  if(BIT_FIELD_32(eax, 8, 4) == 15 || BIT_FIELD_32(eax, 8, 4) == 6){
    info->ModelId = BIT_FIELD_32(eax, 4, 4) +\
                    (BIT_FIELD_32(eax, 16, 4) << 4);
  } else {
    info->ModelId = BIT_FIELD_32(eax, 4, 4);
  }

  /* SteppingId */
  info->SteppingId = BIT_FIELD_32(eax, 0, 4);

}
 
bool CPU_IamBSP(void){
   uint32_t eax, edx;
   CPU_GetMSR(APIC_BASE_MSR, &eax, &edx);
   if((eax & APIC_BASE_MSR_BSP) == APIC_BASE_MSR_BSP) {
      return TRUE;
   } else {
      return FALSE;
   }
}
 
unsigned int CPU_CheckMSR(void)
{
  unsigned int eax, edx, unused; 
  __get_cpuid(1, &eax, &edx, &unused, &unused);
  return edx & CPUID_FLAG_MSR;
}
 
void CPU_GetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
  asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}
 
void CPU_SetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
  asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}



