#ifndef _STDIO_INIT_H_
#define _STDIO_INIT_H_

#include <libk/stddef.h>

typedef ssize_t print_fun_t(const char* dataIn, size_t len);
typedef ssize_t read_fun_t(char* dataOut, size_t len);

void stdio_set_wr_fun(print_fun_t* print_fun);
void stdio_set_rd_fun(read_fun_t* read_fun);

#endif