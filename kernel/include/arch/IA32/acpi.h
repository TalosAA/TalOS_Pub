#ifndef _IA32_ACPI_H_
#define _IA32_ACPI_H_

#include <libk/stdint.h>
#include <libk/stddef.h>
#include <IA32/memdefs.h>

/**
 * ACPI revisions IDs
 */
#define ACPI_REV_1_0        (0)
#define ACPI_REV_2_0        (1)

/**
 * Max number of cores and interrupts
 */
#define MAX_CPU_NUM          (256)
#define MAX_INT_NUM          (256)
#define MAX_NMI_NUM          (16)

/**
 * Max number of I/O APIC supported
 */
#define MAX_IOAPIC_NUM        (16)

/**
 * ACPI Labels
 */
#define RSDP_LABEL            "RSD PTR "
#define RSDT_LABEL            "RSDT"
#define XSDT_LABEL            "XSDT"
#define MADT_LABEL            "APIC"
#define MCFG_LABEL            "MCFG"
#define FADT_LABEL            "FACP"

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
#define MADT_ONLINE_CAPABLE       (1<<1)

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
typedef struct __is_packed {
 char Signature[RSDP_SIGN_LEN];
 uint8_t Checksum;
 char OEMID[OEMID_LEN];
 uint8_t Revision;
 uint32_t RsdtAddress;
} RSDP_hdr_descr_t;

typedef struct __is_packed {
 RSDP_hdr_descr_t firstPart;
 uint32_t Length;
 uint64_t XsdtAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} xSDT_hdr_descr_t;

typedef struct __is_packed {
  acpi_header_t hdr;
  uint32_t PointerToOtherSDT[];
} RSDT_descr_t;

typedef struct __is_packed {
  acpi_header_t hdr;
  uint64_t PointerToOtherSDT[];
} XSDT_descr_t;

/**
 * MADT Lapic header
 */
typedef struct __is_packed {
  uint32_t LAPICAddr;
  uint32_t Flags; /* if eq 1 -> Dual 8259 Legacy PICs Installed*/
} MADT_LAPIC_header_t;

/**
 * MADT Table variable record type definition
 */
typedef struct __is_packed {
  uint8_t EntryType;
  uint8_t Length;
} MADT_var_record_t;

/**
 * MADT Entry Type 0: Processor LAPIC
 */
typedef struct __is_packed {
  uint8_t ProcUID;
  uint8_t LAPIC_Id;
  uint32_t Flags;
} MADT_Proc_LAPIC_t;

/**
 * MADT Entry Type 1: I/O APIC
 */
typedef struct __is_packed {
  uint8_t IOAPIC_ID;
  uint8_t Reserved;
  uint32_t IOAPIC_Addr;
  uint32_t GlobalSysInterBase;
} MADT_IOAPIC_t;

/**
 * MADT Entry Type 2: I/O APIC Interrupt Source Override
 */
typedef struct __is_packed {
  uint8_t BusSource;
  uint8_t IRQSource;
  uint32_t GlobalSysInt;
  uint16_t Flags;
} MADT_IOAPIC_int_src_t;

/**
 * MADT Entry Type 3: I/O APIC NMI Source
 */
typedef struct __is_packed {
  uint8_t Type;
  uint8_t Length;
  uint8_t Flags;
  uint32_t GlobalSysInt;
} MADT_IOAPIC_NMI_Src_t;

/**
 * MADT Entry Type 4: LAPIC NMI
 */
typedef struct __is_packed {
  uint8_t ProcUID;
  uint16_t Flags;
  uint8_t LINTn; /* 0 or 1 */
} MADT_LAPIC_NMI_t;

/**
 * MADT Entry Type 5: LAPIC Address Override
 */
typedef struct __is_packed {
  uint16_t Reserved;
  uint64_t LAPIC_Addr;
} MADT_LAPIC_Addr_Over_t;

/**
 * MADT Entry Type 9: Processor Local x2APIC
 */
typedef struct __is_packed {
  uint16_t Reserved;
  uint32_t ProcessorLocX2APICID;
  uint32_t Flags;
  uint32_t ACPI_ID;
} MADT_x2LAPIC_t;

/**
 * GenericAddressStructure (used in FADT)
 */
typedef struct __is_packed {
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
} GenericAddressStructure;

/**
 * FADT (Fixed ACPI Description Tab) Table structure
 */
typedef struct __is_packed {
    acpi_header_t hdr;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;
    uint8_t  Reserved; /* ACPI 1.0 field no longer used. */
    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;
    uint16_t BootArchitectureFlags; /* reserved in ACPI 1.0; used since ACPI 2.0+ */
    uint8_t  Reserved2;
    uint32_t Flags;
    GenericAddressStructure ResetReg;
    uint8_t  ResetValue;
    uint8_t  Reserved3[3];
    /* 64bit pointers - Available on ACPI 2.0+ */
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;
    GenericAddressStructure X_PM1aEventBlock;
    GenericAddressStructure X_PM1bEventBlock;
    GenericAddressStructure X_PM1aControlBlock;
    GenericAddressStructure X_PM1bControlBlock;
    GenericAddressStructure X_PM2ControlBlock;
    GenericAddressStructure X_PMTimerBlock;
    GenericAddressStructure X_GPE0Block;
    GenericAddressStructure X_GPE1Block;
} FADT_descr_t;

/**
 * DSDT Differentiated System Descriptio Table
 */
typedef struct __is_packed {
    acpi_header_t hdr;
    uint8_t AMLCode[];
} DSDT_descr_t;

/**
 * MCFG PCI configuration space record
 */
typedef struct __is_packed {
  uint64_t BaseAddress;
  uint16_t PCISegmentGroupNum;
  uint8_t  StartPCINumDecodedHostBridge;
  uint8_t  EndPCINumDecodedHostBridge;
  uint32_t Reserved;
} MCFG_PCI_conf_space_t;

typedef struct __is_packed {
  acpi_header_t hdr;
  uint8_t Reserved[8];
  MCFG_PCI_conf_space_t ConfSpaceRecords[];
} MCFG_descr_t;

/**
 * TODO: define other MADT record types if required. 
 */

typedef struct {
  MADT_Proc_LAPIC_t* LAPICs[MAX_CPU_NUM];
  uint32_t LAPICs_Num;
  uint32_t LAPIC_ptr;
  MADT_IOAPIC_int_src_t* IntSrcOverride[MAX_INT_NUM];
  uint32_t IntSrcOverride_Num;
  MADT_IOAPIC_NMI_Src_t* NMI_Source[MAX_NMI_NUM];
  uint32_t NMI_Num;
  MADT_IOAPIC_t* IOAPICs[MAX_IOAPIC_NUM];
  uint32_t IOAPIC_Num;
} MADT_info_t;

bool acpi_tab_checksum(void* tab_addr,
                       size_t size);

RSDP_hdr_descr_t* acpi_get_RSDP_ptr(const uint8_t* addrStart,
                                const uint8_t* addrLimit);

void *acpi_find_table(void *SDTStart, char *tableSignature);

bool acpi_get_MADT_info(RSDP_hdr_descr_t* RSDP_ptr,
                        MADT_info_t* MADT_Info_out);

#endif
