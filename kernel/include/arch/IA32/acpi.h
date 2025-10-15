#ifndef _IA32_ACPI_H_
#define _IA32_ACPI_H_

#include <libk/stdint.h>
#include <libk/stddef.h>
#include <IA32/memdefs.h>

/**
 * Max number of cores and interrupts
 */
#define MAX_CPU_NUM          (256)
#define MAX_INT_NUM           (256)
#define MAX_NMI_NUM           (16)

/**
 * Max number of I/O APIC supported
 */
#define MAX_IOAPIC_NUM        (16)

/**
 * ACPI Labels
 */
#define RSDP_LABEL            "RSD PTR "
#define APIC_LABEL            "APIC"

/**
 * Definitions regarding the ACPI types 
 */
#define ACPI_SIGN_LEN             (4)
#define OEMID_LEN                 (6)
#define OEMID_TAB_ID_LEN          (8)

/**
 * RSDP Table definitions
 */
#define RSDP_SIGN_LEN             (8)

/**
 * MADT Entry Types definitions 
 */
#define MADT_ENT_LAPIC            (0)
#define MADT_ENT_IOAPIC           (1)
#define MADT_ENT_IOAPIC_INT_OVER  (2)
#define MADT_ENT_IOAPIC_NMI_SRC   (3)
#define MADT_ENT_LAPIC_NMI_SRC    (4)
#define MADT_ENT_LAPIC_ADDR_OVER  (5)
#define MADT_ENT_X2APIC           (9)

#define MADT_PROC_ENABLED         (1)

/**
 * ACPI header type
 */
typedef struct __is_packed {
  uint8_t Signature[ACPI_SIGN_LEN];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  uint8_t OEMID[OEMID_LEN];
  uint8_t OEMID_TabId[OEMID_TAB_ID_LEN];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} acpi_header_t;

/**
 * RSDP and xSDT descriptors
 */
typedef struct {
 char Signature[RSDP_SIGN_LEN];
 uint8_t Checksum;
 char OEMID[OEMID_LEN];
 uint8_t Revision;
 uint32_t RsdtAddress;
} __is_packed RSDP_descr_t;

typedef struct {
 RSDP_descr_t firstPart;
 uint32_t Length;
 uint64_t XsdtAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __is_packed xSDT_descr_t;

/**
 * MADT Lapic header
 */
typedef struct {
  uint32_t LAPICAddr;
  uint32_t Flags; /* if eq 1 -> Dual 8259 Legacy PICs Installed*/
} __is_packed MADT_LAPIC_header_t;

/**
 * MADT Table variable record type definition
 */
typedef struct {
  uint8_t EntryType;
  uint8_t Length;
} __is_packed MADT_var_record_t;

/**
 * MADT Record: Processor LAPIC
 */
typedef struct {
  uint8_t LAPIC_ProcId;
  uint8_t LAPIC_Id;
  uint32_t Flags;
} __is_packed MADT_Proc_LAPIC_t;

/**
 * MADT Record: I/O APIC
 */
typedef struct {
  uint8_t IOAPIC_ID;
  uint8_t Reserved;
  uint32_t IOAPIC_Addr;
  uint32_t GlobalSysInterBase;
} __is_packed MADT_IOAPIC_t;

/**
 * MADT Record: I/O APIC Interrupt Source Override
 */
typedef struct {
  uint8_t BusSource;
  uint8_t IRQSource;
  uint32_t GlobalSysInt;
  uint16_t Flags;
} __is_packed MADT_IOAPIC_int_src_t;

/**
 * MADT Record: LAPIC Address Override
 */
typedef struct {
  uint16_t Reserved;
  uint64_t LAPIC_Addr;
} __is_packed MADT_LAPIC_Addr_Over_t;

/**
 * MADT Record: NMI Source
 */
typedef struct {
  uint8_t Type;
  uint8_t Length;
  uint8_t Flags;
  uint32_t GlobalSysInt;
} __is_packed MADT_NMI_Src_t;

/**
 * TODO: define other MADT record types if required. 
 */

typedef struct {
  MADT_Proc_LAPIC_t* LAPICs[MAX_CPU_NUM];
  uint32_t LAPICs_Num;
  uint32_t LAPIC_ptr; 
  MADT_IOAPIC_int_src_t* IntSrcOverride[MAX_INT_NUM];
  uint32_t IntSrcOverride_Num;
  MADT_NMI_Src_t* NMI_Source[MAX_NMI_NUM];
  uint32_t NMI_Num;
  MADT_IOAPIC_t* IOAPICs[MAX_IOAPIC_NUM];
  uint32_t IOAPIC_Num;
} MADT_info_t;

bool acpi_tab_checksum(void* tab_addr,
                       size_t size);

RSDP_descr_t* acpi_get_RSDP_ptr(const uint8_t* addrStart,
                                const uint8_t* addrLimit);

bool acpi_get_MADT_info(RSDP_descr_t* RSDP_ptr,
                        MADT_info_t* MADT_Info_out);

#endif