#include <libk/stdio_init.h>

print_fun_t* libk_stdout_fun;
read_fun_t* libk_stdin_fun;

void stdio_set_wr_fun(print_fun_t* print_fun) {
    libk_stdout_fun = print_fun;
}

void stdio_set_rd_fun(read_fun_t* read_fun) {
    libk_stdin_fun = read_fun;
}
