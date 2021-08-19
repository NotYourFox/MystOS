#include "mem.h"

void* memset(void* ptr, int c, size_t size){ //Fill the certain amount of memory at the provided pointer with the provided value
    char* c_ptr = (char*) ptr; //Creates another pointer to the char
    for (int i = 0; i < size; i++){
        c_ptr[i] = c; //Fill the memory at the address c_ptr+i with c
    }
    return ptr;
}