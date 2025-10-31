#include <libk/limits.h>
#include <libk/stdarg.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <libk/stdio_init.h>
#include <libk/stdlib.h>

extern print_fun_t* libk_stdout_fun;

typedef void* copy_fun_t(void* restrict dest, const void* restrict src, size_t len);

static void* copy_to_stdout(void* restrict dest, const void* restrict src, size_t len) {
  if(libk_stdout_fun((const char*)src, len) < 0){
    //TODO: set errno
    /* Dummy nok return */
    return ((void*)dest + 1);
  }
  /* Dummy ok return */
  return ((void*)dest);
}

static int print_number(char* buff_out, copy_fun_t* cpy_fun,
                        char* str_num, ssize_t precision,
                        ssize_t width, ssize_t zero_pad,
                        size_t max_write, size_t* written, 
                        size_t maxrem) {
  int exit = 0;
  size_t len = strlen(str_num);

  /* print zero pad */
  if(len >= width) {
    zero_pad = 0;
  }
  while(zero_pad > 0){
    char zero = '0';
    if(((*written) + 2) > max_write){
      /* exit */
      return 1;
    }
    if(cpy_fun(buff_out + *written, (void*) &zero, 1) != (buff_out + *written)){
      return -1;
    }
    (*written)++;
    zero_pad--;
    width--;
  }

  /* Manage width */
  if(precision > 0){
    width = width - precision;
    if(str_num[0] == '-'){
      width--;
    }
  } else {
    width = width - len;
  }
  if(width < 0){
    width = 0;
  }
  /* print spaces */
  while(width > 0){
    char space = ' ';
    if(((*written) + 2) > max_write){
      /* exit */
      return 1;
    }
    if(cpy_fun(buff_out + *written, (void*) &space, 1) != (buff_out + *written)){
      return -1;
    } 
    (*written)++;
    width--;
  }

  /* Manage precision */
  if((size_t)precision > len || (str_num[0] == '-' && ((size_t)precision + 1) > len)){
    /* write sign */
    if(str_num[0] == '-'){
      if(((*written) + 2) > max_write){
        /* exit */
        return 1;
      }
      if(cpy_fun(buff_out + *written, (void*) str_num, 1) != (buff_out + *written)){
        return -1;
      } 
      (*written)++;
      str_num++;
      len--;
    }
    precision -= len;
    while(precision > 0){
      char to_write = '0';
      if(((*written) + 2) > max_write){
        break;
      }
      if(cpy_fun(buff_out + *written, (void*) &to_write, 1) != (buff_out + *written)){
        return -1;
      } 
      precision--;
      (*written)++;
    }
  }

  if (maxrem < len) {
    // TODO: Set errno to EOVERFLOW.
    return -1;
  }

  if((*written + len + sizeof(char)) > max_write){
    len = max_write - *written - sizeof(char);
    exit = 1;
  }

  if(cpy_fun(buff_out + *written, (void*) str_num, len) != (buff_out + *written)){
    return -1;
  } 

  *written += len; 

  return exit;
}

static int gen_printf(char* buff_out, copy_fun_t* cpy_fun, size_t n, const char *restrict format, va_list parameters) {
  size_t written = 0;
  int exit = 0;
  const char* format_begun_at = 0;
  ssize_t width = 0;
  ssize_t precision = 0;
  ssize_t zero_pad = 0;

  while ((*format != '\0') && !exit) {
    size_t maxrem = UINT_MAX - written;

    if (format[0] != '%' || format[1] == '%') {
      if (format[0] == '%')
        format++;
      size_t amount = 1;
      while (format[amount] && format[amount] != '%')
        amount++;
      if (maxrem < amount) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if((written + amount + sizeof(char)) > n){
        amount = n - written - sizeof(char);
        exit = true;
      }

      if(cpy_fun(buff_out + written, (const char *)format, amount) != \
        (buff_out + written)){
        return -1;
      }

      format += amount;
      written += amount;
      continue;
    }

    format_begun_at = format++;

    width = 0;
    precision = 0;
    zero_pad = 0;
    if(*format >= 0x30 && *format <= 0x39){
      /* count zeros */
      while (*format == 0x30 && *format != '\0'){ 
        zero_pad++;
        format++;
      }

      /* width field */
      width = atoi(format);
      while (*format >= 0x30 && *format <= 0x39)
        format++;
    }
    if(*format == '.'){
      /* precision field */
      format++;
      precision = atoi(format);
      while (*format >= 0x30 && *format <= 0x39)
        format++;
    }

    /* Format */
    if (*format == 'c') {
      format++;
      char c = (char) va_arg(parameters, int /* char promotes to int */);
      if (!maxrem) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }

      if((written + sizeof(c) + sizeof(char)) > n){
        exit = true;
      } else {

        if(cpy_fun(buff_out + written, &c, sizeof(c)) != (buff_out + written)){
          return -1;
        }

        written++;
      }

    } else if (*format == 's') {
      const char* str = va_arg(parameters, const char*);
      size_t len = strlen(str);
      format++;

      /* Manage width */
      if(len < (size_t)width){
        width -= len;
      }
      /* print spaces */
      while(width > 0){
        char space = ' ';
        if(((written) + 2) > n){
          /* exit */
          exit = 1;
          break;
        }
        if(cpy_fun(buff_out + written, (void*) &space, sizeof(char)) != (buff_out + written)){
          return -1;
        } 
        (written)++;
        width--;
      }

      if (maxrem < len) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }

      if((written + len + sizeof(char)) > n){
        len = n - written - sizeof(char);
        exit = true;
      }

      if(cpy_fun(buff_out + written, (void*) str, len) != (buff_out + written)){
        return -1;
      }

      written += len;
    } else if ((*format == 'l' && *(format + sizeof(char)) != 'l')) {
      char str_num[23];
      memset(str_num, 0, sizeof(str_num));
      format++;

      if(*format == 'd' || *format == 'i') {
        long num = va_arg(parameters, long);
        litoa(num, str_num, 10);
      } else if(*format == 'u'){
        unsigned long num = va_arg(parameters, unsigned long);
        ulitoa(num, str_num, 10);
      } else if(*format == 'x' || *format == 'X') {
        unsigned long num = va_arg(parameters, unsigned long);
        ulitoa(num, str_num, 16);
      }

      format++;

      exit = print_number(buff_out, cpy_fun, str_num, precision, width, \
                          zero_pad, n, &written, maxrem);
      if(exit < 0){
        return exit;
      }

    } else if ((*format == 'l' && *(format + sizeof(char)) == 'l')) {
      char str_num[41];
      memset(str_num, 0, sizeof(str_num));
      format+=2;

      if(*format == 'd' || *format == 'i') {
        long long num = va_arg(parameters, long long);
        litoa(num, str_num, 10);
      } else if(*format == 'u'){
        unsigned long long num = va_arg(parameters, unsigned long long);
        ullitoa(num, str_num, 10);
      } else if(*format == 'x' || *format == 'X') {
        unsigned long long num = va_arg(parameters, unsigned long long);
        ullitoa(num, str_num, 16);
      } 

      format++;

      exit = print_number(buff_out, cpy_fun, str_num, precision, width, \
                          zero_pad, n, &written, maxrem);
      if(exit < 0){
        return exit;
      }

    } else if(*format == 'd' || *format == 'i' || *format == 'u' || \
              *format == 'x' || *format == 'X')
    {
      char str_num[12];
      memset(str_num, 0, sizeof(str_num));

      if(*format == 'd' || *format == 'i') {
        int num = va_arg(parameters, int);
        litoa(num, str_num, 10);
      } else if(*format == 'u'){
        unsigned int num = va_arg(parameters, unsigned int);
        ulitoa(num, str_num, 10);
      } else if(*format == 'x' || *format == 'X') {
        unsigned int num = va_arg(parameters, unsigned int);
        ulitoa(num, str_num, 16);
      } 

      format++;

      exit = print_number(buff_out, cpy_fun, str_num, precision, width, \
                          zero_pad, n, &written, maxrem);
      if(exit < 0){
        return exit;
      }
    
    } else {
      format = format_begun_at;
      size_t len = strlen(format);
      if (maxrem < len) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if((written + len + sizeof(char)) > n){
        len = n - written - sizeof(char);
        exit = true;
      }

      if(cpy_fun(buff_out + written, (void*) format, len) != (buff_out + written)){
        return -1;
      } 

      written += len;
      format += len;
    }
  }

  if(buff_out != NULL){
    buff_out[written] = '\0';
    written++;
  }

  return written;
}

int printf(const char *restrict format, ...){
  int ret = -1;
  va_list parameters;
  va_start(parameters, format);
  ret = gen_printf(NULL, copy_to_stdout, INT_MAX, format, parameters);
  va_end(parameters);
  return ret;
}

int sprintf(char *restrict str, const char *restrict format, ...){
  int ret = -1;
  if(str != NULL) {
    va_list parameters;
    va_start(parameters, format);
    ret = gen_printf(str, memcpy, INT_MAX, format, parameters);
    va_end(parameters);
  }
  return ret;
}

int snprintf(char *restrict str, size_t size, const char *restrict format, ...){
  int ret = -1;
  if(str != NULL) {
    va_list parameters;
    va_start(parameters, format);
    ret = gen_printf(str, memcpy, size, format, parameters);
    va_end(parameters);
  }
  return ret;
}

int vprintf(const char *restrict format, va_list ap){
  return gen_printf(NULL, copy_to_stdout, INT_MAX, format, ap);
}

int vsprintf(char *restrict str, const char *restrict format, va_list ap){
  return gen_printf(str, memcpy, INT_MAX, format, ap);
}

int vsnprintf(char *restrict str, size_t size, const char *restrict format, va_list ap){
  return gen_printf(str, memcpy, size, format, ap);
}