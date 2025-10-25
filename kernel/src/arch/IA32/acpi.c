#include <libk/stddef.h>
#include <libk/string.h>
#include <IA32/acpi.h>

bool acpi_tab_checksum(void* tab_addr, size_t size){
  register uint8_t* index = NULL;
  register uint8_t sum = 0;
  for(index = (uint8_t*)tab_addr; index < (uint8_t*)((size_t)tab_addr + size); index++){
    sum += *index;
  }
  return (sum == 0);
}

RSDP_hdr_descr_t* acpi_get_RSDP_ptr(const uint8_t* addrStart,
                                const uint8_t* addrLimit) {
  register uint8_t* addr_index = (uint8_t*)addrStart;
  RSDP_hdr_descr_t* rsdp_ptr = NULL;

  while(addr_index < addrLimit) {
    if(memcmp(addr_index, RSDP_LABEL, sizeof(RSDP_LABEL) - 1) == 0){
      if(acpi_tab_checksum((void*)addr_index, sizeof(RSDP_hdr_descr_t))){
        rsdp_ptr = (RSDP_hdr_descr_t*)addr_index;
        break;
      }
    }
    addr_index++;
  }
  return rsdp_ptr;
}

void *acpi_find_sdt_table(void *SDTStart, char *tableSignature)
{
  int i;
  int tabsNum = 0;
  bool isXSDT;
  acpi_header_t* retHdr = NULL;
  RSDT_descr_t* RSDT = (RSDT_descr_t*) SDTStart;
  XSDT_descr_t* XSDT = (RSDT_descr_t*) SDTStart;

  if(!memcmp(XSDT->hdr.Signature, XSDT_LABEL, ACPI_SIGN_LEN)) {
    isXSDT = true;
    tabsNum = (RSDT->hdr.Length - sizeof(acpi_header_t)) / sizeof(uint64_t);
  } else {
    isXSDT = false;
    tabsNum = (RSDT->hdr.Length - sizeof(acpi_header_t)) / sizeof(uint32_t);
  }

  for (i = 0; i < tabsNum; i++)
  {
      if(isXSDT) {
        retHdr = (acpi_header_t *) XSDT->PointerToOtherSDT[i];
      } else {
        retHdr = (acpi_header_t *) RSDT->PointerToOtherSDT[i];
      }
      if (!memcmp(retHdr->Signature, tableSignature, ACPI_SIGN_LEN))
          return (void *) retHdr;
  }

  return NULL;
}

bool acpi_get_MADT_info(RSDP_hdr_descr_t* RSDP_ptr, MADT_info_t* MADT_Info_out)
{
  uint8_t* ptr = NULL;
  uint8_t* ptr_end = NULL;
  acpi_header_t* MADT_header = NULL;
  MADT_var_record_t* var_rec = NULL;
  uint32_t* MADT_addr = NULL;
  bool retValue = false;

  /* Clear output info */
  memset(MADT_Info_out, 0, sizeof(MADT_info_t));

  if(RSDP_ptr != NULL && RSDP_ptr->RsdtAddress != 0) {

    MADT_header = (acpi_header_t*)acpi_find_sdt_table(RSDP_ptr->RsdtAddress, MADT_LABEL);

    if(MADT_header != NULL) {
      /* Set LAPIC Address */
      MADT_Info_out->LAPIC_ptr = ((MADT_LAPIC_header_t*)\
                  ((uint32_t)MADT_header + sizeof(acpi_header_t)))->LAPICAddr;

      /* Parse variable length fields */
      ptr = (uint8_t*)((uint32_t)MADT_header +\
                      sizeof(acpi_header_t) +\
                      sizeof(MADT_LAPIC_header_t));
      ptr_end = (uint8_t*)((uint32_t)MADT_header + MADT_header->Length);
      
      while(ptr < ptr_end) {
        var_rec = (MADT_var_record_t*)ptr;
        switch (var_rec->EntryType)
        {
          case MADT_ENT_LAPIC:
            MADT_Info_out->LAPICs[MADT_Info_out->LAPICs_Num] = 
                                  (MADT_Proc_LAPIC_t*)\
                                  ((uint32_t)ptr + sizeof(MADT_var_record_t));
            if(MADT_Info_out->LAPICs[MADT_Info_out->LAPICs_Num]->Flags & 
              (MADT_PROC_ENABLED|MADT_ONLINE_CAPABLE)) {
              MADT_Info_out->LAPICs_Num++;
            }
          break;

          case MADT_ENT_IOAPIC:
            MADT_Info_out->IOAPICs[MADT_Info_out->IOAPIC_Num++] =\
            (MADT_IOAPIC_t*)((uint32_t)ptr + sizeof(MADT_var_record_t));
          break;

          case MADT_ENT_IOAPIC_INT_OVER:
            MADT_Info_out->IntSrcOverride\
            [MADT_Info_out->IntSrcOverride_Num++] = (MADT_IOAPIC_int_src_t*)\
            ((uint32_t)ptr + sizeof(MADT_var_record_t));
          break;

          case MADT_ENT_LAPIC_ADDR_OVER:
            /* Override of the previously obtained pointer */
            MADT_Info_out->LAPIC_ptr = ((MADT_LAPIC_Addr_Over_t*)\
            ((uint32_t)ptr + sizeof(MADT_var_record_t)))->LAPIC_Addr;
          break;

          case MADT_ENT_LAPIC_NMI_SRC:
            MADT_Info_out->NMI_Source[MADT_Info_out->NMI_Num++] =\
            (MADT_NMI_Src_t*)\
            ((uint32_t)ptr + sizeof(MADT_NMI_Src_t));
          break;
          
          default:
            /**
             *  TODO: to parse other entries if it is required
             * */
          break;
        }
        ptr += var_rec->Length;
      }

      retValue = true;
    }
  }
  
  return retValue;
}
