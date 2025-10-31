#include <libk/stdint.h>
#include <libk/stdio.h>
#include <libk/stdlib.h>
#include <libk/string.h>
#include <IA32/io.h>
#include <IA32/pci.h>
#include <coresys/ramfs.h>

static uint16_t pci_read16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t regNum);
static int pci_scanFunction(uint8_t bus, uint8_t dev, uint8_t function);
static int pci_scanDevice(uint8_t bus, uint8_t dev);
static int pci_scanBus(uint8_t bus);
static int pci_ramfs_addnode(uint8_t bus, uint8_t device, uint8_t function);

static inline uint8_t pci_getHeaderType(uint8_t bus, uint8_t dev, uint8_t func) {
  return pci_read16(bus, dev, func, PCI_HDR_TYPE_OFFSET);
}

static inline uint16_t pci_getVendorID(uint8_t bus, uint8_t dev, uint8_t function) {
  return pci_read16(bus, dev, function, PCI_HDR_VENDORID_OFFSET);
}

static inline uint8_t pci_getBaseClass(uint8_t bus, uint8_t dev, uint8_t func) {
  return (uint8_t)(pci_read16(bus, dev, func, PCI_HDR_BASECLASS_OFFSET) & 0xFF);
}

static inline uint8_t pci_getSubClass(uint8_t bus, uint8_t dev, uint8_t func) {
  return (uint8_t)(pci_read16(bus, dev, func, PCI_HDR_SUBCLASS_OFFSET) & 0xFF);
}

static inline uint8_t pci_hdr_01_getSecondaryBus(uint8_t bus, uint8_t dev, uint8_t func) {
  return (uint8_t)(pci_read16(bus, dev, func, PCI_HDR_01_SEC_BUS_OFFSET));
}

static uint16_t pci_read16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t regNum) {
  uint32_t addr;

  /* ENABLE (31) | RESERVED (31:24) | BUS (23:16) | DEV (15:11) | FUNC (10:8) |
   * 32-BIT REG NUM (7:2) | RESERVED (1:0)
   * REG NUM selects the uint32 within the conf. area (base 4 counter); the
   * two LSB of the regNum variable are used to select the uint8 within the
   * uint32.
   */
  addr = (uint32_t)((bus << 16) | (dev << 11) |
                    (func << 8) | (regNum & 0xFC) | 
                    ((uint32_t)PCI_ENABLE_BIT));
  outl(addr, PCI_CONFIG_ADDRESS);

  /* Select the uint16 required (uint8 based addressing)*/
  return (uint16_t)(inl(PCI_CONFIG_DATA) >> ((regNum & 0x03) << 3)) & 0xFFFF;
}

static int pci_scanFunction(uint8_t bus, uint8_t dev, uint8_t function) {
  uint8_t baseClass;
  uint8_t subClass;
  uint8_t secondaryBus;

  baseClass = pci_getBaseClass(bus, dev, function);
  subClass = pci_getSubClass(bus, dev, function);
  if(pci_getVendorID(bus, dev, function) != PCI_NOT_VALID_VENDOR_ID) {

    if(pci_ramfs_addnode(bus, dev, function) == RET_T_NOK) {
      return RET_T_NOK;
    }

    if ((baseClass == PCI_BRIDGE_BASECLASS) && 
        (subClass == PCI_TO_PCI_BRIDGE_SUBCLASS)) {
        /* PCI to PCI bridege */
        secondaryBus = pci_hdr_01_getSecondaryBus(bus, dev, function);
        pci_scanBus(secondaryBus);
    }
  }
  return RET_T_OK;
}

static int pci_scanDevice(uint8_t bus, uint8_t dev) {
  uint8_t func = 0;
  uint16_t vendorID = PCI_NOT_VALID_VENDOR_ID;
  uint8_t hdrType = 0;

  vendorID = pci_getVendorID(bus, dev, func);
  if (vendorID != PCI_NOT_VALID_VENDOR_ID){
    if(pci_scanFunction(bus, dev, func) != RET_T_OK) {
      return RET_T_NOK;
    }
    hdrType = pci_getHeaderType(bus, dev, func);
    if((hdrType & PCI_HDR_IS_MULTIFUNCTION) != 0){
      /* Multi-function dev */
      for(func = 1; func < PCI_FUNC_PER_DEV; func++){
        if(pci_scanFunction(bus, dev, func) != RET_T_OK) {
          return RET_T_NOK;
        }
      }
    }
  }
  return RET_T_OK;
}

static int pci_ramfs_addnode(uint8_t bus, uint8_t device, uint8_t function) {
  char pciDevName [MAX_FNAME];
  sprintf(pciDevName, "0000:%02u:%02u.%02u", bus, device, function);
  if(ramfs_mkDir(pciDevName, "/sys/device/pci", NULL) == NULL) {
    /* the files should not exist at this point */
    return RET_T_NOK;
  }
  return RET_T_OK;
}

static int pci_scanBus(uint8_t bus) {
  uint8_t dev;

  for (dev = 0; dev < PCI_DEV_PER_BUS; dev++) {
      if(pci_scanDevice(bus, dev) != RET_T_OK) {
        return RET_T_NOK;
      }
  }
  return RET_T_OK;
}

int pci_scan(void) {
  uint8_t function;
  uint8_t bus = 0;
  uint8_t headerType;
  int fserr = RET_T_NOK;

  if(ramfs_mkDir("pci", "/sys/device", &fserr) == NULL){
    if(fserr != RFS_FILE_ALREADY_EXISTS){
      return RET_T_NOK;
    }
    if(ramfs_deleteNode("/sys/device/pci") != RFS_OK){
      return RET_T_NOK;
    }
    if(ramfs_mkDir("pci", "/sys/device", NULL) != RFS_OK){
      return RET_T_NOK;
    }
  }

  headerType = pci_getHeaderType(0, 0, 0);
  if ((headerType & PCI_HDR_IS_MULTIFUNCTION) == 0) {
    /* System with a single PCI host controller */
    if(pci_scanBus(0) != RET_T_OK) {
          return RET_T_NOK;
    }
  } else {
    /* System with multiple PCI host controllers */
    for (function = 0; function < PCI_FUNC_PER_DEV; function++) {
        if (pci_getVendorID(0, 0, function) == PCI_NOT_VALID_VENDOR_ID){
          break;
        }
        bus = function;
        if(pci_scanBus(bus) != RET_T_OK) {
          return RET_T_NOK;
        }
    }
  }
  return RET_T_OK;
}