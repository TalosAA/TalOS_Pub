#include <libk/stdint.h>
#include <devs/keyboard.h>
#include <IA32/utils.h>
#include <IA32/io.h>
#include <coresys/vfs.h>


#define KB_PORT       0x60
#define KB_BUFF_LEN   256

static kb_press_t   KB_Buff[KB_BUFF_LEN] = {0};
static uint16_t     lastKey = 0;
static uint16_t     head = 0;
static uint16_t     count = 0;

void KB_IRQ(void){
  kb_press_t ScanCode;
  ScanCode = inb(KB_PORT);

  if(count != KB_BUFF_LEN){
    KB_Buff[lastKey] = ScanCode;
    lastKey = (lastKey + 1) % KB_BUFF_LEN;
    count++;
  }

}

kb_press_t KB_Get_Key(void){

  kb_press_t ScanCode;

  if(count == 0)
    return 0;

  ScanCode = KB_Buff[head];
  head = (head + 1) % KB_BUFF_LEN;
  count--;

  return ScanCode;

}


// ret_t keyboard_fmgr(__is_unused void* res_spec_descr, fio_open_mode_t open_mode, void* buf, size_t* count){
//   ret_t ret = FIO_NOK;
//   size_t i = 0;

//   if(open_mode == FIO_DATA_READ){
//       for(i = 0; i < *count; i++){
//         ((char*)&buf)[i] = 0;
//         while(((char*)&buf)[i] == 0)
//           ((char*)&buf)[i] = (char) KB_Get_Key();
//       }
//       ret = FIO_OK;
//   }

//   return ret;
// }