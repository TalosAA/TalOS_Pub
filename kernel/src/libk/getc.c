#include <libk/stdio.h>
#include <libk/stdio_init.h>

extern read_fun_t* libk_stdin_fun;

int getchar(void){
    char in = 0;
    int ret = 0;

    if(libk_stdin_fun(&in, sizeof(in)) != -1){
        ret = (unsigned char)in;
    }

    return ret;
}

char *gets(char *s){
    unsigned int count = 0;
    char in = '\0';
    do {
        in = getchar();
        s[count] = in;
        count++;
    } while(in != '\0' && in != '\n');
    return s;
}