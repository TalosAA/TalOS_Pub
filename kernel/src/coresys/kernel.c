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
#include <libk/kermsg.h>
#include <hal/interrupts.h>
#include <hal/conswitch.h>
#include <hal/mmap.h>
#include <hal/init.h>
#include <hal/platform.h>
#include <hal/bootloader.h>
#include <coresys/kernel.h>
#include <coresys/vfs.h>
#include <coresys/initrd.h>
#include <coresys/ramdisk.h>
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
  set_read_fun(stdin_fun);

  /* Initialize vga stdout */
  vga_initialize();
  vga_cursor_off();
  stdio.write = (fs_write_fun_t)vga_fs_write;
  stdio.length = 0;
  stdio.impl_def = 0;

  /* Initialize serial stdout */
  // stdio.write = serial_fs_write;
  // stdio.length = 0;
  // stdio.impl_def = COM1;

  set_write_fun(stdout_fun);

}

void test_int(uint32_t id, uint32_t EIP, uint32_t CS, uint32_t EFLAGS) {
  printf("id 0x%x eip 0x%x cs 0x%x eflags 0x%x\n", id, EIP, CS, EFLAGS);
}

//TODO
bootloader_module_t* modules;
uint32_t nModules;

void kernel_init_fs(void) {
  uint32_t i = 0;
  uint8_t* initrdPtr = NULL;
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
  
  LoadInitrd((uint8_t*)modules[i].mod_start);
  mem_unmap(modules[i].mod_start, modules[i].mod_size);
  fs_set_root(getRoot());
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

  kernel_init_fs();

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
  char testPath[] = "/etc/temp1.txt";
  char* node = NULL;
  char* tokenCon = NULL;

  kernel_init();
  printf("Start Kernel\n");

  //Read /etc/temp1.txt
  fs_node_t* test1 = fs_get_node(NULL, "/etc/temp1.txt");
  char test[255];
  fs_read(test1, test, 10, 0);

  printf("%s\n", test);

  dirent_t entry;
  dirent_t* retEntry;

  i = 0;
  do {
    fs_readdir(fs_get_node(NULL, "/dev/../etc"), i, &entry, &retEntry);
    if(retEntry != NULL)
      printf("%s\n", retEntry->d_name);
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