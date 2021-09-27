#ifndef VGAIO_H
#define VGAIO_H

#include <stdint.h>
#include "moskernel.h"

void clear();
void printc(const char* str, char color);
void print(const char* str);
size_t strlen(const char* str);
int isdigit(char c);
int strtoint(char c);
size_t strnlen(const char* str, int max);
char* strcpy(char* dest, const char* src);
char* strcat (char* str1, char* str2);
char* tolower(char* str);
char toupper(char* str);
int find(char* dest, const char* src);

#endif
