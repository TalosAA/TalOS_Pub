#ifndef _PCI_H_
#define _PCI_H_

#include <libk/stdint.h>
#include <libk/stddef.h>
#include <coresys/vfs.h>

/**
 * PCI topology definitions
 */
#define PCI_BUS_NUM                  (256)
#define PCI_DEV_PER_BUS              (32)
#define PCI_FUNC_PER_DEV             (8)

/**
 * PCI I/O ports
 */
#define PCI_CONFIG_ADDRESS           (0xCF8)
#define PCI_CONFIG_DATA              (0xCFC)
#define PCI_ENABLE_BIT               (1<<31)

/**
 * PCI headers definitions
 */
#define PCI_HDR_CLASS_CODE_SIZE      (3)
#define PCI_HDR_IS_MULTIFUNCTION     (0x80)

#define PCI_BRIDGE_BASECLASS         (0x06)
#define PCI_TO_PCI_BRIDGE_SUBCLASS   (0x04)

/**
 * Invalid vendor ID.
 */
#define PCI_NOT_VALID_VENDOR_ID      (0xFFFF)

/** 
 * PCI common header definitions
 */
#define PCI_HDR_VENDORID_OFFSET       (0x00)
#define PCI_HDR_DEVICEID_OFFSET       (0x02)
#define PCI_HDR_COMMAND_OFFSET        (0x04)
#define PCI_HDR_STATUS_OFFSET         (0x06)
#define PCI_HDR_REVISION_ID_OFFSET    (0x08)
#define PCI_HDR_CLASS_CODE_OFFSET     (0x09)
#define PCI_HDR_REG_LV_PROG_INTERF    (0x09)
#define PCI_HDR_SUBCLASS_OFFSET       (0x0A)
#define PCI_HDR_BASECLASS_OFFSET      (0x0B)
#define PCI_HDR_CACHELINE_SIZE_OFFSET (0x0C)
#define PCI_HDR_LATENCY_TIME_OFFSET   (0x0D)
#define PCI_HDR_TYPE_OFFSET           (0x0E)
#define PCI_HDR_BITS_OFFSET           (0x0F)

typedef struct __is_packed {
  uint16_t VendorID;
  uint16_t DeviceID;
  uint16_t Command;
  uint16_t Status;
  uint8_t  RevisionID;
  uint8_t  ClassCode[PCI_HDR_CLASS_CODE_SIZE];
  uint8_t  CachelineSize;
  uint8_t  LatencyTimer;
  uint8_t  HeaderType;
  uint8_t  BITS;
} PCI_common_header;

/** 
 * PCI header type 00 definitions
 */
#define PCI_HDR_00_BASE_ADDR_REGS_N   (6)
#define PCI_HDR_00_RESERVED_SIZE      (7)
#define PCI_HDR_00_BAR_OFFSET         (sizeof(PCI_common_header))
#define PCI_HDR_00_CBUS_CIS_OFFSET    (PCI_HDR_00_BAR_OFFSET +\
                                       PCI_HDR_00_BASE_ADDR_REGS_N * 4)
#define PCI_HDR_00_SUBSYS_VENDOR_OFFSET (PCI_HDR_00_CBUS_CIS_OFFSET + 4)
#define PCI_HDR_00_SUBSYS_ID_OFFSET    (PCI_HDR_00_SUBSYS_VENDOR_OFFSET + 2)
#define PCI_HDR_00_EXROMADDR_OFFSET    (PCI_HDR_00_SUBSYS_ID_OFFSET + 2)
#define PCI_HDR_00_CAPABILITIES_OFFSET (PCI_HDR_00_SUBSYS_ID_OFFSET + 4)
#define PCI_HDR_00_INT_LINE_OFFSET     (PCI_HDR_00_CAPABILITIES_OFFSET +\
                                        PCI_HDR_00_RESERVED_SIZE)
#define PCI_HDR_00_INT_PIN_OFFSET      (PCI_HDR_00_INT_LINE_OFFSET + 1)
#define PCI_HDR_00_MIN_GRANT_OFFSET    (PCI_HDR_00_INT_PIN_OFFSET + 1)
#define PCI_HDR_00_MAX_LAT_OFFSET      (PCI_HDR_00_MIN_GRANT_OFFSET + 1)

typedef struct __is_packed {
  PCI_common_header commhdr;
  uint32_t BaseAddressRegisters[PCI_HDR_00_BASE_ADDR_REGS_N];
  uint32_t CardbusCISPtr;
  uint16_t SubsystemVendorID;
  uint16_t SubsystemID;
  uint32_t ExpansionROMBaseAddr;
  uint8_t  CapabilitiesPtr;
  uint8_t  Reserved[PCI_HDR_00_RESERVED_SIZE];
  uint8_t  InterruptLine;
  uint8_t  InterruptPin;
  uint8_t  Min_Grant;
  uint8_t  Max_Latency;
} PCI_type_00_hdr;

/* PCI-to-PCI bridge header (type 0x1)*/

#define PCI_HDR_01_BASE_ADDR_REGS_N  (2)
#define PCI_HDR_01_RESERVED_SIZE     (3)
#define PCI_HDR_01_SEC_BUS_OFFSET    (sizeof(PCI_common_header) + 9)

typedef struct __is_packed {
  PCI_common_header commhdr;
  uint32_t BaseAddressRegisters[PCI_HDR_01_BASE_ADDR_REGS_N];
  uint8_t  PrimaryBusNumber;
  uint8_t  SecondaryBusNumber;
  uint8_t  SubordinateBusNumber;
  uint8_t  SecondaryLatencyTimer;
  uint8_t  IOBase;
  uint8_t  IOLimit;
  uint16_t SecondaryStatus;
  uint16_t MemoryBase;
  uint16_t MemoryLimit;
  uint16_t PrefetchableMemBase;
  uint16_t PrefetchableMemLimit;
  uint32_t PrefetchableBaseUpp32bits;
  uint32_t PrefetchableLimitUpp32bits;
  uint16_t IOBaseUpp16Bits;
  uint16_t IOLimitUpp16Bits;
  uint8_t  CapabilityPtr;
  uint8_t  Reserved[PCI_HDR_01_RESERVED_SIZE];
  uint32_t ExpansionROMBaseAddr;
  uint8_t  InterruptLine;
  uint8_t  InterruptPIN;
  uint16_t BridgeControl;
} PCI_type_01_hdr;

/* PCI-to-CardBus bridge header (type 0x2)*/
typedef struct __is_packed {
  PCI_common_header commhdr;
  uint32_t CardBusSockBaseAddr;
  uint8_t  OffCapabilitiesList;
  uint8_t  Reserved;
  uint16_t SecondaryStatus;
  uint8_t  PCIBusNumber;
  uint8_t  CardBusBusNum;
  uint8_t  SubBusNum;
  uint8_t  CardBusLatencyTimer;
  uint32_t MemoryBaseAddr0;
  uint32_t MemoryLimit0;
  uint32_t MemoryBaseAddr1;
  uint32_t MemoryLimit1;
  uint32_t IOBaseAddr0;
  uint32_t IOLimit0;
  uint32_t IOBaseAddr1;
  uint32_t IOLimit1;
  uint8_t  InterruptLine;
  uint8_t  InterruptPIN;
  uint16_t BridgeControl;
  uint16_t SubsysDevID;
  uint16_t SubSysVendorID;
  uint32_t PCCardLegacyModAddr16;
} PCI_type_02_hdr;

int pci_scan(void);

ssize_t pci_vfs_confing_read(struct fs_node* node,
                             void* buf,
                             size_t nbyte,
                             off_t offset);

ssize_t pci_vfs_confing_write(struct fs_node* node,
                              const void* buf,
                              size_t nbyte,
                              off_t offset);

#endif