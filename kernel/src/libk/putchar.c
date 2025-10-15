#include <libk/stdio.h>
#include <libk/stdio_init.h>

extern print_fun_t* libk_stdout_fun;
extern void* libk_out_param;

int putchar(int ic) {
  char c = (char) ic;
  libk_stdout_fun(&c, sizeof(c));
  return ic;
}
