#ifndef _PS2_KEYB_H_
#define _PS2_KEYB_H_

#include <coresys/vfs.h>

typedef unsigned char kb_press_t;


void KB_IRQ(void);
kb_press_t KB_Get_Key(void);
// ret_t keyboard_fmgr(__is_unused void* res_spec_descr, fio_open_mode_t data_dir, void* buf, size_t* count);

#endif
