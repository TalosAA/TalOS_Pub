#ifndef _STRING_H
#define _STRING_H 1

#include <libk/stddef.h>

int memcmp(const void*, const void*, size_t);
void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
errno_t memset_s(void *dest, rsize_t destsz, int c, rsize_t len);
size_t strlen(const char*);
size_t strnlen(const char *s, size_t maxlen);
char *strcpy(char * dst, const char * src);
char * strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strtok_r(char* str, const char* delim, char** saveptr);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);

#endif
