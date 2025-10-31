#include <stdint.h>
#include <string.h>

char *strncpy(char *dest, const char *src, size_t n) {
  size_t i = 0;

  for (i = 0; i < n && src[i] != '\0'; i++) 
    dest[i] = src[i];
  if(i < n)
    dest[i] = '\0';

  return dest;
}

char *strcpy(char *dest, const char *src) {
  return strncpy(dest, src, SIZE_MAX);
}

size_t strlen(const char *str) { return strnlen(str, SIZE_MAX); }

size_t strnlen(const char *str, size_t maxlen) {
  size_t len = 0;
  while (str[len] && len < maxlen) len++;
  return len;
}

int strcmp(const char *s1, const char *s2) {
  return strncmp(s1, s2, SIZE_MAX);
}

int strncmp(const char *s1, const char *s2, size_t n) {
  char* s1_in = (char*)s1;
  char* s2_in = (char*)s2;
  while(*s1_in != 0 && (*s1_in == *s2_in) && n > 0) {
    s1_in++;
    s2_in++;
    n--;
  }
  return ((int)*s1_in) - ((int)*s2_in);
}

char *strtok_r(char* str, const char* delim, char** saveptr) {
  size_t count = 0;
  size_t delimLen = 0;
  char* token = NULL;

  if(str != NULL || *saveptr != NULL) {
    delimLen = strlen(delim);

    if(str != NULL) {
      token = str;
    } else {
      token = *saveptr;
    }
    
    while(memcmp(token, delim, delimLen) == 0)
      token+=delimLen;

    while((memcmp(&token[count], delim, delimLen) != 0) && token[count] != '\0') {
      count++;
    }

    if(token[count] != '\0') {
      *saveptr = &token[count + delimLen];
      token[count] = '\0';
    } else { /* end of string */
      *saveptr = NULL;
      if(token[0] == '\0')
        token = NULL;
    }
  }

  return token;
}

char *strcat(char *dest, const char *src){
  return strncat(dest, src, SIZE_MAX);
}

char* strncat(char *dest, const char *src, size_t n)
{
  size_t dest_len = strlen(dest);
  size_t i;

  for (i = 0; i < n && src[i] != '\0'; i++)
      dest[dest_len + i] = src[i];
  dest[dest_len + i] = '\0';

  return dest;
}