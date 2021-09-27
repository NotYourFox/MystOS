#include "vgaio.h"
#include "status.h"

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
    if (chr == '\n'){ // new line
        col = 0;
        row++;
        return;
    }
    if (chr == '\r'){ // carriage return (without clearing)
        col = 0;
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

size_t strnlen(const char* str, int max){
    int len = 0;
    for (len = 0; len < max; len++){
        if (str[len] == 0){
            break;
        }
    }
    return len;
} 

// ⣿⣿⣷⡁⢆⠈⠕⢕⢂⢕⢂⢕⢂⢔⢂⢕⢄⠂⣂⠂⠆⢂⢕⢂⢕⢂⢕⢂⢕⢂
// ⣿⣿⣿⡷⠊⡢⡹⣦⡑⢂⢕⢂⢕⢂⢕⢂⠕⠔⠌⠝⠛⠶⠶⢶⣦⣄⢂⢕⢂⢕
// ⣿⣿⠏⣠⣾⣦⡐⢌⢿⣷⣦⣅⡑⠕⠡⠐⢿⠿⣛⠟⠛⠛⠛⠛⠡⢷⡈⢂⢕⢂
// ⠟⣡⣾⣿⣿⣿⣿⣦⣑⠝⢿⣿⣿⣿⣿⣿⡵⢁⣤⣶⣶⣿⢿⢿⢿⡟⢻⣤⢑⢂
// ⣾⣿⣿⡿⢟⣛⣻⣿⣿⣿⣦⣬⣙⣻⣿⣿⣷⣿⣿⢟⢝⢕⢕⢕⢕⢽⣿⣿⣷⣔
// ⣿⣿⠵⠚⠉⢀⣀⣀⣈⣿⣿⣿⣿⣿⣿⣿⣿⣿⣗⢕⢕⢕⢕⢕⢕⣽⣿⣿⣿⣿
// ⢷⣂⣠⣴⣾⡿⡿⡻⡻⣿⣿⣴⣿⣿⣿⣿⣿⣿⣷⣵⣵⣵⣷⣿⣿⣿⣿⣿⣿⡿
// ⢌⠻⣿⡿⡫⡪⡪⡪⡪⣺⣿⣿⣿⣿⣿⠿⠿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃
// ⠣⡁⠹⡪⡪⡪⡪⣪⣾⣿⣿⣿⣿⠋⠐⢉⢍⢄⢌⠻⣿⣿⣿⣿⣿⣿⣿⣿⠏⠈
// ⡣⡘⢄⠙⣾⣾⣾⣿⣿⣿⣿⣿⣿⡀⢐⢕⢕⢕⢕⢕⡘⣿⣿⣿⣿⣿⣿⠏⠠⠈
// ⠌⢊⢂⢣⠹⣿⣿⣿⣿⣿⣿⣿⣿⣧⢐⢕⢕⢕⢕⢕⢅⣿⣿⣿⣿⡿⢋⢜⠠⠈
// ⠄⠁⠕⢝⡢⠈⠻⣿⣿⣿⣿⣿⣿⣿⣷⣕⣑⣑⣑⣵⣿⣿⣿⡿⢋⢔⢕⣿⠠⠈
// ⠨⡂⡀⢑⢕⡅⠂⠄⠉⠛⠻⠿⢿⣿⣿⣿⣿⣿⣿⣿⣿⡿⢋⢔⢕⢕⣿⣿⠠⠈
// ⠄⠪⣂⠁⢕⠆⠄⠂⠄⠁⡀⠂⡀⠄⢈⠉⢍⢛⢛⢛⢋⢔⢕⢕⢕⣽⣿⣿⠠⠈


int isdigit(char c) {
    return c >= 48 && c <= 57;
}

int strtoint(char c){
    if (!(isdigit(c))){
        return -EINVARG;
    }
    return c - 0x30;
}

char* strcpy(char* dest, const char* src){
    char* res = dest;
    while (*src != 0){
        *dest = *src;
        src += 1;
        dest += 1;
    }
    *dest = 0x00;
    return res;
}

void print(const char* str){ // Default print (light gray)
    size_t len = strlen(str);
    for (int i = 0; i < len; i++){
        print_char(str[i], 7);
    }
}

void printc(const char* str, char color){ // Colored print
    size_t len = strlen(str);
    for (int i = 0; i < len; i++){
        print_char(str[i], color);
    }
}
// VGA COLORS TABLE
//
// value | color
//-------+-----------------
//   0   | BLACK
//   1   | BLUE
//   2   | GREEN
//   3   | CYAN
//   4   | RED
//   5   | MAGENTA
//   6   | BROWN
//   7   | LIGHT GRAY
//   8   | DARK GRAY
//   9   | LIGHT BLUE
//   10  | LIGHT GREEN
//   11  | LIGHT CYAN
//   12  | LIGHT RED
//   13  | LIGHT MAGENTA
//   14  | YELLOW
//   15  | WHITE
//
// https://www.fountainware.com/EXPL/vga_color_palettes.htm


void clear(){
    video_mem = (uint16_t*)(0xB8000);
    row = 0;
    col = 0;
    for (int y = 0; y < VGA_HEIGHT; y++){
        for(int x = 0; x < VGA_WIDTH; x++){
            put_char(x, y, ' ', 0);
        }
    }
}