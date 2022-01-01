#ifndef VGAIO_H
#define VGAIO_H

#include <stdint.h>
#include <stdarg.h>
#include "moskernel.h"

void clear();
int is_linefeed();
size_t strlen(const char* str);
int isdigit(char c);
int strtoint(char* str);
int strtoint_char(char c);
char* inttostr(int num);
size_t intlen(int num);
size_t strnlen(const char* str, int max);
char* strcpy(char* dest, const char* src);
char* strcat (int num, ...);
char* tolower (char* str);
char* toupper (char* str);
int find(char* dest, const char* src);
int strnlen_term(const char* str, int max, char terminator);
int strncmp(const char* str1, const char* str2, int n);
int istrncmp(const char* str1, const char* str2, int n);
char char_toupper(char c);
char char_tolower(char c);
char* strrev(char* str);
char* strcut(const char* sid, const char* fid);
void log(int num, int res, ...);
char* hex(long decimalnum);
void sprint(const char* str);
void vprint(int num, va_list argptr);
void sprintc(const char* str, char color);
void vprintc(int num, char color, va_list argptr);
void printc(int num, char color, ...);
void print(int num, ...);

#endif
