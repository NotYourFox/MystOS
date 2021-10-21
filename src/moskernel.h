#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

void kernel_main();
void panic(const char* msg);

#define error(value) (void*)value
#define error_int(value) (int)value
#define is_error(value) ((int)value < 0)

#endif