#ifndef VGAPRINT_H
#define VGAPRINT_H

#include <stdint.h>
#include "moskernel.h"

void clear();
void printc(const char* str, char color);
void print(const char* str);

#endif