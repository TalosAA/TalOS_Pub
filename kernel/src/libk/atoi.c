#include <libk/stdlib.h>

#define ASCII_0           ('0')
#define ASCII_9           ('9')
#define ASCII_TO_DEC(C)   (C - ASCII_0)

static const int dec_pow[] = \
{1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};


int atoi(const char *in){
  unsigned char digits = 0;
  unsigned char i = 0;
  int to_ret = 0;

  /* Digits count */
  while(in[digits] != 0 && digits < 10){
    if(in[digits] >= ASCII_0 && in[digits] <= ASCII_9){
      digits++;
    } else {
      break;
    }
  }

  if(digits != 0){
    for(i = 0; i < digits; i++){
      to_ret += ASCII_TO_DEC(in[digits - i - 1])*(dec_pow[i]);
    }
  }

  return to_ret;
}