#include <libk/stdint.h>
#include <libk/stdio.h>
#include <libk/stdlib.h>
#include <libk/string.h>
#include <libk/kcleenup.h>
#include <libk/kheap.h>
#include <IA32/io.h>
#include <IA32/pci.h>
#include <coresys/vfs.h>
#include <coresys/ramfs.h>

/**
 * Descriptor used to access the right PCI peripheral
 * during read/write file accesses.
 */
typedef struct {
  uint8_t bus;
  uint8_t device;
  uint8_t function;
} pci_ramfs_conf_area_t;

#define PCI_DEV_DIR_NAME_LEN  (sizeof("0000:00:00.00"))
#define PCI_FS_PATH           "/sys/device/pci"

static int pci_scanFunction(uint8_t bus, uint8_t dev, uint8_t function);
static int pci_scanDevice(uint8_t bus, uint8_t dev);
static int pci_scanBus(uint8_t bus);
static fs_node_t* pci_ramfs_addNewConfAreaFile(const char* fileName,
                                               const char* parentPath,
                                               uint8_t bus,
                                               uint8_t device,
                                               uint8_t function,
                                               int *err);
static int pci_ramfs_addFiles(const char* pciPeriphDir, 
                              uint8_t bus,
                              uint8_t device,
                              uint8_t function);

static inline uint32_t pci_read32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t regNum) {
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
  return inl(PCI_CONFIG_DATA) >> (((regNum & 0x03) << 3) & 0xFF);
}

static inline void pci_write32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t regNum, uint32_t in) {
  uint32_t addr;

  addr = (uint32_t)((bus << 16) | (dev << 11) |
                    (func << 8) | (regNum & 0xFC) | 
                    ((uint32_t)PCI_ENABLE_BIT));
  outl(addr, PCI_CONFIG_ADDRESS);
  outl(in, PCI_CONFIG_DATA);
}

static inline void pci_write16(uint8_t bus, uint8_t dev, uint8_t func, uint8_t regNum, uint16_t in) {
  uint32_t addr;

  addr = (uint32_t)((bus << 16) | (dev << 11) |
                    (func << 8) | (regNum & 0xFC) | 
                    ((uint32_t)PCI_ENABLE_BIT));
  outl(addr, PCI_CONFIG_ADDRESS);
  outw(in, PCI_CONFIG_DATA);
}

static inline void pci_write8(uint8_t bus, uint8_t dev, uint8_t func, uint8_t regNum, uint8_t in) {
  uint32_t addr;

  addr = (uint32_t)((bus << 16) | (dev << 11) |
                    (func << 8) | (regNum & 0xFC) | 
                    ((uint32_t)PCI_ENABLE_BIT));
  outl(addr, PCI_CONFIG_ADDRESS);
  outb(in, PCI_CONFIG_DATA);
}

static inline uint8_t pci_getHeaderType(uint8_t bus, uint8_t dev, uint8_t func) {
  return (uint8_t)(uint16_t)pci_read32(bus, dev, func, PCI_HDR_TYPE_OFFSET);
}

static inline uint16_t pci_getVendorID(uint8_t bus, uint8_t dev, uint8_t function) {
  return (uint16_t)pci_read32(bus, dev, function, PCI_HDR_VENDORID_OFFSET);
}

static void pci_scanCommonHdr(uint8_t bus, 
                              uint8_t dev,
                              uint8_t function,
                              PCI_common_header* hdrOut){
  hdrOut->VendorID = pci_getVendorID(bus, dev, function);
  hdrOut->DeviceID = (uint16_t)pci_read32(bus, dev, function, PCI_HDR_DEVICEID_OFFSET);
  hdrOut->Command = (uint16_t)pci_read32(bus, dev, function, PCI_HDR_COMMAND_OFFSET);
  hdrOut->Status = (uint16_t)pci_read32(bus, dev, function, PCI_HDR_STATUS_OFFSET);
  hdrOut->RevisionID = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_REVISION_ID_OFFSET);
  hdrOut->ClassCode[0] = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_REG_LV_PROG_INTERF);
  hdrOut->ClassCode[1] = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_SUBCLASS_OFFSET);
  hdrOut->ClassCode[2] = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_BASECLASS_OFFSET);
  hdrOut->CachelineSize = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_CACHELINE_SIZE_OFFSET);
  hdrOut->LatencyTimer = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_LATENCY_TIME_OFFSET);
  hdrOut->HeaderType = pci_getHeaderType(bus, dev, function);
  hdrOut->BITS = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_BITS_OFFSET);
}

static void pci_scanType00Hdr(uint8_t bus, 
                              uint8_t dev,
                              uint8_t function,
                              PCI_type_00_hdr* hdrOut){
  register int i;
  register int barOff;
  pci_scanCommonHdr(bus, dev, function, (PCI_common_header*)hdrOut);
  for(i = 0; i < PCI_HDR_00_BASE_ADDR_REGS_N; i++) {
    barOff = PCI_HDR_00_BAR_OFFSET + sizeof(uint32_t) * i;
    hdrOut->BaseAddressRegisters[i] = pci_read32(bus, dev, function, barOff);
  }
  hdrOut->CardbusCISPtr = pci_read32(bus, dev, function, PCI_HDR_00_CBUS_CIS_OFFSET);
  hdrOut->SubsystemVendorID = (uint16_t)pci_read32(bus, dev, function, PCI_HDR_00_SUBSYS_VENDOR_OFFSET);
  hdrOut->SubsystemID = (uint16_t)pci_read32(bus, dev, function, PCI_HDR_00_SUBSYS_ID_OFFSET);
  hdrOut->ExpansionROMBaseAddr = pci_read32(bus, dev, function, PCI_HDR_00_EXROMADDR_OFFSET);
  hdrOut->CapabilitiesPtr = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_00_CAPABILITIES_OFFSET);
  hdrOut->InterruptLine = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_00_INT_LINE_OFFSET);
  hdrOut->InterruptPin = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_00_INT_PIN_OFFSET);
  hdrOut->Min_Grant = (uint8_t)pci_read32(bus, dev, function, PCI_HDR_00_MIN_GRANT_OFFSET);
  hdrOut->Max_Latency= (uint8_t)pci_read32(bus, dev, function, PCI_HDR_00_MAX_LAT_OFFSET);
}

static inline uint8_t pci_getBaseClass(uint8_t bus, uint8_t dev, uint8_t func) {
  return (uint8_t)pci_read32(bus, dev, func, PCI_HDR_BASECLASS_OFFSET);
}

static inline uint8_t pci_getSubClass(uint8_t bus, uint8_t dev, uint8_t func) {
  return (uint8_t)pci_read32(bus, dev, func, PCI_HDR_SUBCLASS_OFFSET);
}

static inline uint8_t pci_hdr_01_getSecondaryBus(uint8_t bus, uint8_t dev, uint8_t func) {
  return (uint8_t)pci_read32(bus, dev, func, PCI_HDR_01_SEC_BUS_OFFSET);
}

/**
 * RAMFS PCI support functions
 */
static int pci_ramfs_addnode(uint8_t bus, uint8_t device, uint8_t function) {
  char pciDevName[PCI_DEV_DIR_NAME_LEN];
  char pciDevPath[sizeof(PCI_FS_PATH) + PCI_DEV_DIR_NAME_LEN];
  sprintf(pciDevName, "0000:%02u:%02u.%02u", bus, device, function);
  if(ramfs_mkDir(pciDevName, PCI_FS_PATH, NULL) == NULL) {
    /* the files should not exist at this point */
    return RET_T_NOK;
  }

  sprintf(pciDevPath, "%s/%s", PCI_FS_PATH, pciDevName);

  if(pci_ramfs_addFiles(pciDevPath, bus, device, function) == RET_T_NOK) {
    return RET_T_NOK;
  }

  return RET_T_OK;
}

static fs_node_t* pci_ramfs_addNewConfAreaFile(const char* fileName,
                                               const char* parentPath,
                                               uint8_t bus,
                                               uint8_t device,
                                               uint8_t function,
                                               int *err) {
  CLEENUP_FUNCTION_SET(kfree, free);

  ramfs_file_header_t* ramfs_node = NULL;
  fs_node_t* parentDir = NULL;

  if(err != NULL) {
    *err = VFS_T_NOK;
  }

  if(fs_checkFileName(fileName, NULL) != RET_T_OK) {
    CLEENUP(default);
  }

  parentDir = fs_get_node(NULL, parentPath);
  if(parentDir == NULL){
    CLEENUP(default);
  }

  if(fs_find(parentDir, fileName) != NULL){
    if(err != NULL) {
      *err = RFS_FILE_ALREADY_EXISTS;
    }
    CLEENUP(default);
  }

  ramfs_node = kmalloc(sizeof(ramfs_file_header_t));
  if(ramfs_node == NULL){ 
    CLEENUP(default);
  }

  ramfs_node->children = NULL;
  ramfs_node->next = NULL;
  ramfs_node->fs_node.open = NULL;
  ramfs_node->fs_node.close = NULL;
  ramfs_node->fs_node.read = pci_vfs_confing_read;
  ramfs_node->fs_node.write = pci_vfs_confing_write;
  ramfs_node->fs_node.find = NULL;
  ramfs_node->fs_node.readdir = NULL;
  ramfs_node->fs_node.length = 0;
  ramfs_node->fs_node.ino = fs_new_ino_id();
  strncpy(ramfs_node->fs_node.name, fileName, MAX_FNAME);
  ramfs_node->fs_node.owner_group = ramfs_node->fs_node.owner_id = 0;
  ramfs_node->fs_node.perm = 0;
  ramfs_node->fs_node.type = FTYPE_REGULAR;
  ramfs_node->fs_node.node_ptr = NULL;
  ramfs_node->fs_node.impl_def = 0;

  ramfs_node->filePtr = kmalloc(sizeof(pci_ramfs_conf_area_t));
  if(ramfs_node->filePtr == NULL) {
    CLEENUP(free_ramfs_node);
  }

  ((pci_ramfs_conf_area_t*)ramfs_node->filePtr)->bus = bus;
  ((pci_ramfs_conf_area_t*)ramfs_node->filePtr)->device = device;
  ((pci_ramfs_conf_area_t*)ramfs_node->filePtr)->function = function;

  ((ramfs_file_header_t*)ramfs_getLastChild(parentDir))->next = ramfs_node;

  if(err != NULL) {
    *err = VFS_T_OK;
  }

  return (fs_node_t*)ramfs_node;

  CLEENUP_LABEL(free_ramfs_node, free, ramfs_node);
  CLEENUP_DEFAULT_LABEL();

  return NULL;

}

ssize_t pci_vfs_confing_read(struct fs_node* node, void* buf, size_t nbyte, off_t offset) {
  ramfs_file_header_t* ramfsNode = (ramfs_file_header_t*)node;
  pci_ramfs_conf_area_t* device = ramfsNode->filePtr;
  uint8_t* readBuff8 = (uint8_t*) buf;
  uint32_t* readBuff32 = (uint32_t*) buf;
  size_t readNum = (nbyte/sizeof(uint32_t));
  size_t readRem = nbyte % 4;
  size_t i, k;

  for(i = 0; i < readNum; i++) {
    readBuff32[i * sizeof(uint32_t)] = pci_read32(device->bus, device->device, device->function, offset + i * sizeof(uint32_t));
  }
  for(k = 0; k < readRem; k++) {
    readBuff8[k] = (uint8_t)pci_read32(device->bus, device->device, device->function, offset + i * sizeof(uint32_t) + k);
  }
  
  return (readNum * sizeof(uint32_t) + readRem);
}

ssize_t pci_vfs_confing_write(struct fs_node* node, const void* buf, size_t nbyte,
                      off_t offset) {
  ramfs_file_header_t* ramfsNode = (ramfs_file_header_t*)node;
  pci_ramfs_conf_area_t* device = ramfsNode->filePtr;
  ssize_t writtenBytes = 0;
  uint16_t toWrite = 0;

  /* Currently only Command register and Status Register are writable (in aligned mode) */
  if(nbyte != 0 && (offset == PCI_HDR_COMMAND_OFFSET || offset == PCI_HDR_STATUS_OFFSET)){
    if(nbyte >= 2) {
      toWrite = *((uint16_t*)buf);
      pci_write16(device->bus, device->device, device->function, offset, toWrite);
    } else {
      toWrite = *((uint16_t*)buf) & 0xFF;
      pci_write16(device->bus, device->device, device->function, offset, (uint8_t)toWrite);
    }
    writtenBytes = nbyte;
  }
  
  return writtenBytes;
}

static int pci_ramfs_addFiles(const char* pciPeriphDir, 
                              uint8_t bus,
                              uint8_t device,
                              uint8_t function) {
  uint16_t vendorID = pci_getVendorID(bus, device, function);
  uint16_t deviceID = (uint16_t)pci_read32(bus, device, function, PCI_HDR_DEVICEID_OFFSET);
  fs_node_t* fileNodePtr = NULL;
  if((fileNodePtr = ramfs_newRegFile("vendor", pciPeriphDir, NULL)) == NULL){
    return RET_T_NOK;
  }
  if(ramfs_write(fileNodePtr, &vendorID, sizeof(vendorID), 0) == 0){
    return RET_T_NOK;
  }
  if((fileNodePtr = ramfs_newRegFile("device", pciPeriphDir, NULL)) == NULL){
    return RET_T_NOK;
  }
  if(ramfs_write(fileNodePtr, &deviceID, sizeof(deviceID), 0) == 0){
    return RET_T_NOK;
  }
  if((fileNodePtr = ramfs_newRegFile("subsystem_vendor", pciPeriphDir, NULL)) == NULL){
    return RET_T_NOK;
  }
  if(ramfs_write(fileNodePtr, &deviceID, sizeof(deviceID), 0) == 0){
    return RET_T_NOK;
  }
  if((fileNodePtr = pci_ramfs_addNewConfAreaFile("config", pciPeriphDir, bus, device, function, NULL)) == NULL){
    return RET_T_NOK;
  }

  //TODO: add new files

  return RET_T_OK;
}

/**
 * RAMFS PCI recursive scan functions (Legacy PCI 2.0 mode)
 */
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
    if(ramfs_deleteNode("/sys/device/pci") != VFS_T_OK){
      return RET_T_NOK;
    }
    if(ramfs_mkDir("pci", "/sys/device", NULL) == NULL){
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