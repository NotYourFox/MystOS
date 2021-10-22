#ifndef VGAIO_H
#define VGAIO_H

#include <stdint.h>
#include "moskernel.h"

void clear();
int is_linefeed();
void printc(const char* str, char color);
void print(const char* str);
size_t strlen(const char* str);
int isdigit(char c);
int strtoint(char c);
char* inttostr(int num);
size_t intlen(int num);
size_t strnlen(const char* str, int max);
char* strcpy(char* dest, const char* src);
char* strcat (const char* str1, const char* str2);
char* tolower (char* str);
char* toupper (char* str);
int find(char* dest, const char* src);
int strnlen_term(const char* str, int max, char terminator);
int strncmp(const char* str1, const char* str2, int n);
int istrncmp(const char* str1, const char* str2, int n);
char char_toupper(char c);
char char_tolower(char c);
void reverse(char* str);

#endif
