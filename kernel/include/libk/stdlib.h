#ifndef _STDLIB_H_
#define _STDLIB_H_

int atoi(const char *nptr);
unsigned int atou(const char *nptr);
void itoa(int input, char *buffer, int base);
void uitoa(unsigned int input, char *buffer, int base);
void litoa(long input, char *buffer, int base);
void ulitoa(unsigned long input, char *buffer, int base);
void llitoa(long long input, char *buffer, int base);
void ullitoa(unsigned long long input, char *buffer, int base);

#endif
