#include <libk/stdio_init.h>

print_fun_t* libk_stdout_fun;
read_fun_t* libk_stdin_fun;

void set_write_fun(print_fun_t* print_fun) {
    libk_stdout_fun = print_fun;
}

void set_read_fun(read_fun_t* read_fun) {
    libk_stdin_fun = read_fun;
}
