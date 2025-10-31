#include <libk/stdio_init.h>
#include <libk/stdio.h>
#include <libk/stdint.h>
#include <coresys/timer.h>
#include <spinlock.h>
#include <devs/vga.h>
#include <devs/keyboard.h>
#include <devs/serial.h>
#include <libk/queue.h>
#include <libk/time.h>
#include <libk/kerrmsg.h>
#include <hal/interrupts.h>
#include <hal/conswitch.h>
#include <hal/mmap.h>
#include <hal/init.h>
#include <hal/platform.h>
#include <hal/bootloader.h>
#include <coresys/kernel.h>
#include <coresys/vfs.h>
#include <coresys/initrd.h>
#include <coresys/ramfs.h>
#include <coresys/types.h>
#include <libk/kheap.h>
#include <libk/string.h>

/* This is a test file containing the kernel main */

os_data_t OSData __OS_DATA;
uint8_t kheap[KER_HEAP_SIZE];

extern void Serial_IRQ(void);

void kernel_init(void);
void kernel_main(void);

serial_conf_t serial0;
fs_node_t stdio;

ssize_t stdout_fun(const char* dataIn, size_t len) {
  return fs_write(&stdio, dataIn, len, 0);
}

ssize_t stdin_fun(char* dataOut, size_t len) {
  return fs_read(&stdio, dataOut, len, 0);
}

void kernel_wellcome_msg(void){

  printf("Wellcome to %s!\n", OS_NAME);
  printf("Version: %s\n", OS_VERSION);

}

void InitStdio(void) {

  /* Initialize Serial0 */
  serial0.serial_port = COM1;
  serial0.fifo_mode_en = true;
  serial0.irq_en = true;
  serial0.baud_rate_div = SERIAL_SET_BAUD_38400;
  serial0.parity = SERIAL_NO_PARITY;
  serial0.stop_bits = 1;
  serial0.flow_control = SERIAL_FLOW_CTL_RTS_CTS;
  serial_init(&serial0);

  /* std input */
  stdio.read = (fs_read_fun_t)serial_fs_read;
  stdio_set_rd_fun(stdin_fun);

  /* Initialize vga stdout */
  // vga_initialize();
  // vga_cursor_off();
  // stdio.write = (fs_write_fun_t)vga_fs_write;
  // stdio.length = 0;
  // stdio.impl_def = 0;

  /* Initialize serial stdout */
  stdio.write = (fs_write_fun_t)serial_fs_write;
  stdio.length = 0;
  stdio.impl_def = COM1;

  stdio_set_wr_fun(stdout_fun);

}

void test_int(uint32_t id, uint32_t EIP, uint32_t CS, uint32_t EFLAGS) {
  printf("id 0x%x eip 0x%x cs 0x%x eflags 0x%x\n", id, EIP, CS, EFLAGS);
}

//TODO
bootloader_module_t* modules;
uint32_t nModules;

void kernel_initramfs(void) {
  ramfs_init();
  ramfs_mkDir("initrd", "/", NULL);
  ramfs_mkDir("sys", "/", NULL);
  ramfs_mkDir("proc", "/sys", NULL);
  ramfs_mkDir("device", "/sys", NULL);
}

void kernel_init_fs(void) {
  uint32_t i = 0;

  kernel_initramfs();

  bl_GetModules(&modules, &nModules);
  if(nModules == 0) {
    while (1)
    {
      //TODO PANIC
    }
  }
  for(i = 0; i < nModules; i++) {
    modules[i].mod_start = mem_map(modules[i].mod_start, modules[i].mod_start, modules[i].mod_size, false, true, NULL);
    if(*((uint32_t*)modules[i].mod_start) == UINT32_INITRD_ORDER(INITRD_MAGIC)){
      break;
    }
    mem_unmap(modules[i].mod_start, modules[i].mod_size);
  }
  if(i == nModules) {
    while (1)
    {
      //TODO PANIC
    }
  }

  ramfs_loadInitrd((uint8_t*)modules[i].mod_start, "/initrd");
  mem_unmap(modules[i].mod_start, modules[i].mod_size);
}

void kernel_init(void) {
  char boot_date_buff[ASCTIME_BUFF_LEN + 1] = { 0 };
  struct tm boot_date;

  InitKernelHeap(kheap, KER_HEAP_SIZE);

  /* Initialize stdio of the kernel */
  InitStdio();
 
  /* Starting Kernel BOOT */
  kernel_wellcome_msg();

  /* Initialize System HW */
  SetSysTimerFrequency(SYS_TICK_FREQ_HZ);

  kernel_init_fs();

  InitSys();

  /* Get the discovered configuration */
  GetSysConfiguration(&OSData.SystemConfig);

  /* Initialize Kernel time management */
  InitKernelTimers();

  /* Get current timestamp */
  GetSysTimestamp(&OSData.timestamp);
  ts_to_date(OSData.timestamp, &boot_date);
  asctime_r(&boot_date, boot_date_buff);
  printf("Booting time: %s\n", boot_date_buff);

}

extern void kmem_change_map_attributes (void* address,
                                  size_t pagesNum,
                                  bool isWritable,
                                  bool isCacheble,
                                  bool isUserland);



void kernel_main(void) {
  char* wait[] = {"<*    >", "< *   >", "<  *  >", "<   * >", "<    *>",
                  "<   * >", "<  *  >", "< *   >" };
  unsigned char i = 0;
  char key = 0;
  mem_map_t memMap;
  uint32_t availMem = 0;
  char* node = NULL;
  char* tokenCon = NULL;

  kernel_init();
  printf("Start Kernel\n");

  //Create and write ramfs file
  fs_node_t* testwr = ramfs_newRegFile("testwr.txt", "/initrd/test_fill", NULL);
  char testwBuf[] = "testwr";
  fs_write(testwr, testwBuf, strlen(testwBuf), 0);
  char testrBuf[50] = {0};
  fs_read(testwr, testrBuf, 4, 0);
  printf("test read1: %s\n", testrBuf);
  memset(testrBuf,0,sizeof(testrBuf));
  fs_read(testwr, testrBuf, 2, 4);
  printf("test read2: %s\n", testrBuf);
  strcpy(testrBuf, "viva pippo!!");
  fs_write(testwr, testrBuf, strlen(testrBuf), 0);
  fs_read(testwr, testrBuf, strlen(testrBuf), 0);
  printf("test read3: %s\n", testrBuf);

  //Read /etc/temp1.txt
  // fs_node_t* test1 = fs_get_node(NULL, "/initrd/test_fill/temp1.txt");
  // char test[255];
  // fs_read(test1, test, 10, 0);

  // printf("%s\n", test);

  dirent_t entry;
  dirent_t* retEntry;

  printf("/\n", retEntry->d_name);
  i = 0;
  do {
    fs_readdir(fs_get_node(NULL, "/"), i, &entry, &retEntry);
    if(retEntry != NULL)
      printf("    %s\n", retEntry->d_name);
    i++;
  } while(retEntry != NULL);

  printf("/sys\n", retEntry->d_name);
  i = 0;
  do {
    fs_readdir(fs_get_node(NULL, "/sys"), i, &entry, &retEntry);
    if(retEntry != NULL)
      printf("    %s\n", retEntry->d_name);
    i++;
  } while(retEntry != NULL);

  printf("  /sys/device\n", retEntry->d_name);
  i = 0;
  do {
    fs_readdir(fs_get_node(NULL, "/sys/device"), i, &entry, &retEntry);
    if(retEntry != NULL)
      printf("      %s\n", retEntry->d_name);
    i++;
  } while(retEntry != NULL);

  printf("    /sys/device/pci\n", retEntry->d_name);
  i = 0;
  do {
    fs_readdir(fs_get_node(NULL, "/sys/device/pci"), i, &entry, &retEntry);
    if(retEntry != NULL)
      printf("        %s\n", retEntry->d_name);
    i++;
  } while(retEntry != NULL);


  printf("Memory mapping:\n");
  GetMemoryMap(&memMap);

  for(i = 0; i < memMap.areasNum; i++) {
    printf("Start Addr: 0x%.08x | Length: 0x%.08x\n",
        memMap.areas[i].start, memMap.areas[i].length);
    availMem += memMap.areas[i].length;
  }
  printf("Available memory %d MB.\n", availMem/(1024*1024));

  i = 0;

  /* Dummy wait */
  while(1){
    printf("%s\r", wait[i%(sizeof(wait)/sizeof(wait[0]))]);
    i++;
    Wait_ms(100);
    if((key = KB_Get_Key()) != 0){
      printf(" \r\n");
      do {
        printf("%c", key);
      } while((key = KB_Get_Key()) != 0);
      printf("\n");
    }
  }

  while(1);

}