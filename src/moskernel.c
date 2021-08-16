#include "moskernel.h"
#include "idt/idt.h"
#include "io/io.h"

//WRITING A PRINT FUNCTION IN VGA MODE

//The video memory starts at 0xB8000 for colored displays and 0xB0000
//for monochrome displays. We should load the video_mem pointer there.

//The simple array variable is a pointer to the first array object.  
//The array index is just an offset in memory from that pointer.
//So we can act with pointers like with an arrays, where the pointer
//is the first object of an array

//Each char uses 2 bytes: a char byte and a color byte, so we should
//put the symbol into the first of 2 bytes in the video memory, and
//color into the second:
//uint16_t* video_mem = (uint16_t*)(0xB8000);
//video_mem[0] = char;
//video_mem[1] = color;
//which we can easily simplify to 0x<color_byte><char_byte> (respect
//the endianness of x86!) For example: print('A', 3) will do:
//video_mem[0] = 0x0341;

//Let's write a make_char() function that will load a char into the
//video memory. What it will do is return a flipped char code.

//First of all, it shifts the color variable by 8 bits. 
//Assume that the color is now 0x12 (0001 0010 in binary), and the 
//symbol is 0x41 ('A'). After shifting, it will become 
//0001 0010 0000 0000b, that equals 0x1200.

//So, the color code shifted 8 bits, leaving two zeros behind
//The first part is complete. Now we need to set the symbol code. To
//do it, we need to simply OR it with our symbol code, so the empty
//bytes will be filled with it. The function result will be 0x1241.

uint16_t* video_mem = 0;
uint16_t col = 0;
uint16_t row = 0;

uint16_t make_char(char chr, char color){
    return color << 8 | chr;
}

void put_char(int x, int y, char chr, char color){
    video_mem[(y * VGA_WIDTH) + x] = make_char(chr, color);
}

void print_char(char chr, char color){
    if (chr == '\n'){
        col = 0;
        row++;
        return;
    }
    put_char(col, row, chr, color);
    col++;
    if (col >= VGA_WIDTH) {
        col = 0;
        row++;
    }
}

size_t strlen(const char* str){
    size_t len = 0;
    while(str[len]){
        len++;
    }
    return len;
}

void print(const char* str, char color){
    size_t len = strlen(str);
    for (int i = 0; i < len; i++){
        print_char(str[i], color);
    }
}

void term_init(){
    video_mem = (uint16_t*)(0xB8000);
    row = 0;
    col = 0;
    for (int y = 0; y < VGA_HEIGHT; y++){
        for(int x = 0; x < VGA_WIDTH; x++){
            put_char(x, y, ' ', 0);
        }
    }
}

void kernel_main(){
    term_init();
    print("Starting MystOS...", 15);
    idt_init();
}