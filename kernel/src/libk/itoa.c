#include <libk/stdlib.h>
#include <libk/stddef.h>

#define MAX_INT_DEC_DIGIT (10)
#define ASCII_0           ('0')
#define ASCII_A           ('A')

#define BASE_MIN          (2)
#define BASE_MAX          (32)


static inline void reverse_buffer(char* buf, size_t len) {
  char temp = 0;
  size_t i = 0;

  for(i = 0; i < len/2; i++){
    temp = buf[i];
    buf[i] = buf[len - i - 1];
    buf[len - i - 1] = temp;
  }

}

void ulitoa(unsigned long input, char *buffer, int base) {
  unsigned char count = 0;
  unsigned long n = input;
  unsigned int rem = 0;

  do {
    rem = n % base;
    n = n / base;
    if(rem <= 9){ 
      buffer[count++] = rem + ASCII_0;
    } else if(rem > 9 && rem < BASE_MAX){ 
      buffer[count++] = rem - 10 + ASCII_A;
    } else {
      break;
    }
  } while(n != 0);
  reverse_buffer(buffer, count);
}

void ullitoa(unsigned long long input, char *buffer, int base) {
  unsigned char count = 0;
  unsigned long long n = input;
  unsigned int rem = 0;

  do {
    rem = n % base;
    n = n / base;
    if(rem <= 9){ 
      buffer[count++] = rem + ASCII_0;
    } else if(rem > 9 && rem < BASE_MAX){ 
      buffer[count++] = rem - 10 + ASCII_A;
    } else {
      break;
    }
  } while(n != 0);
  reverse_buffer(buffer, count);
}

void litoa(long input, char *buffer, int base) {
  unsigned char count = 0;
  long n = input;

  if(input < 0){
    n = n * (-1);
    buffer[count++] = '-';
  }
  ulitoa((unsigned long)n, &buffer[count], base);
}

void llitoa(long long input, char *buffer, int base) {
  unsigned char count = 0;
  long long n = input;

  if(input < 0){
    n = n * (-1);
    buffer[count++] = '-';
  }
  ullitoa((unsigned long long)n, &buffer[count], base);
}

void itoa(int input, char *buffer, int base) {
  unsigned char count = 0;
  int n = input;

  if(input < 0){
    n = n * (-1);
    buffer[count++] = '-';
  }
  uitoa((unsigned long)n, &buffer[count], base);
}

void uitoa(unsigned int input, char *buffer, int base) {
  ulitoa((unsigned long)input, buffer, base);
}
