#include <libk/stdint.h>

void test_userland(void);

/* NOTA: la directory delle pagine deve essere accessibile in ring 3 (user)*/

extern void*  BSP_stack_top;
extern uint32_t* page_directory;
extern uint32_t* kernel_page_tables;
uint8_t testStackKernel[0x2000] __attribute__((aligned(0x1000)));
uint8_t testStackUser[0x2000];

void jump_usr_code_example() {
  kmem_change_map_attributes((void*)((uint32_t)testStackKernel), 2, true, true, true);
  kmem_change_map_attributes((void*)((uint32_t)testStackUser), 2, true, true, true);
  kmem_change_map_attributes(&test_userland, 1, true, false, true);
  kmem_change_map_attributes(page_directory, 1, true, false, true);
  kmem_change_map_attributes(kernel_page_tables, 2, true, false, true);

  SetSysCallsStackAddress(testStackKernel, 0x2000);

  JumpToUserCode(testStackUser, 0x2000, &test_userland);
}

void test_userland(void) {
    //printf("Hello from userland!\n");
    while(1);
}