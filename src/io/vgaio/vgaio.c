#include "vgaio.h"
#include "status.h"
#include "mem/heap/kheap.h"
#include "config.h"
#include "mem/mem.h"

#define VIDEO_MEMORY_START_ADDR 0xB8000

/*
WRITING A PRINT FUNCTION IN VGA MODE

The video memory starts at 0xB8000 for colored displays and 0xB0000
for monochrome displays. We should load the video_mem pointer there.

The simple array variable is a pointer to the first array object.  
The array index is just an offset in memory from that pointer.
So we can act with pointers like with an arrays, where the pointer
is the first object of an array

Each char uses 2 bytes: a char byte and a color byte, so we should
put the symbol into the first of 2 bytes in the video memory, and
color into the second:
uint16_t* video_mem = (uint16_t*)(0xB8000);
video_mem[0] = char;
video_mem[1] = color;
which we can easily simplify to 0x<color_byte><char_byte> (respect
the endianness of x86!) For example: print('A', 3) will do:
video_mem[0] = 0x0341;

Let's write a make_char() function that will load a char into the
video memory. What it will do is return a flipped char code.

First of all, it shifts the color variable by 8 bits. 
Assume that the color is now 0x12 (0001 0010 in binary), and the 
symbol is 0x41 ('A'). After shifting, it will become 
0001 0010 0000 0000b, that equals 0x1200.

So, the color code shifted 8 bits, leaving two zeros behind
The first part is complete. Now we need to set the symbol code. To
do it, we need to simply OR it with our symbol code, so the empty
bytes will be filled with it. The function result will be 0x1241.
*/

uint16_t* video_mem = 0; //We will set it in clear() function.
uint16_t col = 0;
uint16_t row = 0;

uint16_t make_char(char chr, char color){
    return color << 8 | chr;
}

void put_char(int x, int y, char chr, char color){
    video_mem[(y * VGA_WIDTH) + x] = make_char(chr, color);
}

int check_screen_overflow(){
    if (row == VGA_HEIGHT){
        return 1;
    } else {
        return 0;
    }
}

void clear_row(int i){
    for(int x = 0; x < VGA_WIDTH; x++){
        put_char(x, i, ' ', 7);
    }
}

void copy_row(int dest, int src){
    int row_bytes = 2 * VGA_WIDTH; //bytes in 1 row
    void* res = kzalloc(row_bytes);
    char color;
    char c;
    int x = 0;
    memcpy(res, video_mem + (src * VGA_WIDTH), row_bytes);
    for (int i = 0; i < row_bytes; i += 2){
        c = *((char*)res + i);
        color = *((char*)res + i + 1);
        put_char(x, dest, c, color);
        x++;
    }
}

void scroll_down_on_screen_overflow(){
    for (int i = 0; i < VGA_HEIGHT; i++){
        clear_row(i);
        if (i != VGA_HEIGHT - 1){
            copy_row(i, i + 1);
        }
    }
    row -= 1;
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

int is_linefeed(){
    return col == 0;
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

int strnlen_term(const char* str, int max, char terminator){
    int len = 0;
    for (len = 0; len < max; len++){
        if (str[len] == 0 || str[len] == terminator){
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
    return c >= 0x30 && c <= 0x39;
}

int strtoint_char(char c){
    if (!(isdigit(c))){
        return -EINVARG;
    }
    return c - 0x30;
}

int pow(int n, int exp){
    int res = 1;
    for (int i = exp; i > 0; i--){
        res *= n;
    }
    return res;
}

int strtoint(char* str){
    char* s = str;
    int isNegative = 0;
    if (*s == '-'){
        isNegative = 1;
    }
    int exp = strlen(str) - 1;
    int multiplier;
    int res = 0;
    int resc;
    while (*s++ != 0x00){
        multiplier = pow(10, exp);
        resc = strtoint_char(*s);
        if (resc < 0){
            return -EINVARG;
        }
        res += resc * multiplier;
        exp--;
    }
    if (isNegative){
        res *= -1;
    }
    return res;
}

char* strrev(char* str){
    char* res = str;
    char ch;
    int len = strlen(str);
    for (int i = 0; i < len/2; i++){
        ch = res[i];
        res[i] = res[len-1-i];
        res[len-1-i] = ch;
    }
    return res;
}

size_t intlen(int num){
    int res = 0;
    if (num == 0){
        res = 1;
    }

    while (num != 0){
        res++;
        num = num / 10;
    }
    return res;
}

char* strcut(const char* csid, const char* cfid){
    char* sid = (char*) csid;
    char* fid = (char*) cfid;
    char* res;
    while(sid++ != fid){
        *res++ = *sid;
    }
    return res;
}

char* inttostr(int num){
    int i = 0;
    char* str = kzalloc(intlen(num) + 1);
    int isNegative = 0;
  
    if (num == 0){
        str[i++] = '0';
        str[i] = 0x00;
        return str;
    }

    if (num < 0){
        isNegative = 1;
        num = -num;
    }

    while (num != 0){
        int rem = num % 10;
        str[i++] = rem + 0x30;
        num = num / 10;
    }

    if (isNegative){
        str[i++] = '-';
    }

    str[i] = 0x00;
    str = strrev(str);
    return str;
}

char* strcpy(char* dest, const char* src){
    char* res = dest;
    while (*src != 0){
        *dest = *src;
        src++;
        dest++;
    }
    *dest = 0x00;
    return res;
}

int is_upper(char c){
    return c >= 0x41 && c <= 0x5A;
}

int is_lower(char c){
    return c >= 0x61 && c <= 0x7A;
}

char char_tolower(char c){
    if(is_upper(c)){
        return c + 0x20;
    }
    return c;
}

char char_toupper(char c){
    if(is_lower(c)){
        return c - 0x20;
    }
    return c;
}

char* tolower(char* str){
    for (int i = 0; i < strlen(str); i++){
        str[i] = char_tolower(str[i]);
    }
    return str;
}

char* toupper(char* str){
    for (int i = 0; i < strlen(str); i++){
        str[i] = char_toupper(str[i]);
    }
    return str;
}

char* sstrcat(const char* dest, const char* src){
    char* res = kzalloc(strlen(dest) + strlen(src) + 1);
    char* str1 = strcpy(res, dest);
    char* ptr = str1 + strlen(str1);
    while (*src != 0x00) {
        *ptr++ = *src++;
    }
    *ptr = 0x00;
    return res;
}

char* vstrcat(int num, va_list argptr){
    const char* src;
    src = va_arg(argptr, const char*);
    char* res = (char*) src;
    num -= 1;
    for (int i = 0; i < num; i++){
       res = sstrcat((const char*) res, va_arg(argptr, const char*));
    }
    return res;
}

char* strcat(int num, ...){
    va_list argptr;
    va_start(argptr, num);
    char* res = vstrcat(num, argptr);
    va_end(argptr);
    return res;
}

int strncmp(const char* str1, const char* str2, int n){
    unsigned char u1, u2;

    while (n-- > 0){
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2){
            return u1 - u2;
        }
        if (u1 == '\0'){
            return 0;
        }
    }
    return 0;
}

int istrncmp(const char* str1, const char* str2, int n){
    unsigned char u1, u2;

    while (n-- > 0){
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2 && char_tolower(u1) != char_tolower(u2)){
            return u1 - u2;
        }
        if (u1 == '\0'){
            return 0;
        }
    }
    return 0;
}

int find_char(char* str, const char c){
    int i;
    for (i = 0; i < strlen(str); i++){
        if (str[i] == c){
            return i;
        }
    }
    return -1;
}

int find(char* dest, const char* src){
    char* ptr = dest;
    int res = find_char(ptr, src[0]);
    if (res < 0){
        return -1;
    }
    ptr += res;
    if (strlen(ptr) < strlen(src)){
        res = -1;
        return -1;
    }
    for (int i = 0; i < strlen(src); i++){
        if (ptr[i] != src[i]){
            res = find(ptr + i, src);
            if (res < 0){
                return -1;
            }
        }
    }
    return res;
}

char* hex(long decimalnum){
    char* res;
    if (decimalnum == 0){
        res = "0x00";
        goto out;
    }
    long quotient, remainder;
    int i = 0;
    char hexadecimalnum[100];
    char* tmp;
    quotient = decimalnum;
    while (quotient != 0){
        remainder = quotient % 16;
        if (remainder < 10){
            hexadecimalnum[i++] = 48 + remainder;
        } else {
            hexadecimalnum[i++] = 55 + remainder;
        }
        quotient = quotient / 16;
    }
    if (strlen(hexadecimalnum) == 1){
        tmp = strcat(2, "0", hexadecimalnum);
    } else {
        tmp = strrev(hexadecimalnum);
    }
    res = strcat(2, "0x", tmp);
out:
    return res;
}

void sprint(const char* str){
    size_t len = strlen(str);
    for (int i = 0; i < len; i++){
        print_char(str[i], 7);
    }
    // if (check_screen_overflow()){
    //     scroll_down_on_screen_overflow();
    // }
}

void sprintc(const char* str, char color){
    size_t len = strlen(str);
    for (int i = 0; i < len; i++){
        print_char(str[i], color);
    }
    // if (check_screen_overflow()){
    //     scroll_down_on_screen_overflow();
    // }
}

void vprint(int num, va_list argptr){
    const char* str;
    for (int i = 0; i < num; i++){
        str = va_arg(argptr, const char*);
        sprint(str);
    }
    // if (check_screen_overflow()){
    //     scroll_down_on_screen_overflow();
    // }
}

void vprintc(int num, char color, va_list argptr){
    const char* str;
    for (int i = 0; i < num; i++){
        str = va_arg(argptr, const char*);
        sprintc(str, color);
    }
    // if (check_screen_overflow()){
    //     scroll_down_on_screen_overflow();
    // }
}

void print(int num, ...){ // Default print (light gray)
    if (check_screen_overflow()){
       scroll_down_on_screen_overflow();
    }
    va_list argptr;
    va_start(argptr, num);
    vprint(num, argptr);
    va_end(argptr);
}

void printc(int num, char color, ...){ // Colored print
    if (check_screen_overflow()){
        scroll_down_on_screen_overflow();
    }
    va_list argptr;
    va_start(argptr, color);
    vprintc(num, color, argptr);
    va_end(argptr);
}

void log(int num, int res, ...){
    va_list argptr;
    va_start(argptr, res);
    switch(res){
        case LOG_OK:
            printc(1, vga_green, "[+] ");
            vprint(num, argptr);
            sprint("\n");
            break;
        case LOG_ERR:
            printc(1, vga_red, "[-] ");
            vprint(num, argptr);
            sprint("\n");
            break;
        case LOG_WARN:
            printc(1, vga_yellow, "[!] ");
            vprint(num, argptr);
            sprint("\n");
            break;
        case LOG_NOTICE:
            printc(1, vga_lightblue, "[=] ");
            vprint(num, argptr);
            sprint("\n");
            break;
    }
    va_end(argptr);
}

void clear(){
    video_mem = (uint16_t*)(VIDEO_MEMORY_START_ADDR);
    row = 0;
    col = 0;
    for (int y = 0; y < VGA_HEIGHT; y++){
        for(int x = 0; x < VGA_WIDTH; x++){
            put_char(x, y, ' ', 0);
        }
    }
}
