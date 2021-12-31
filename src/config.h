#ifndef CONFIG_H
#define CONFIG_H

#define MYSTOS_INT_TOTAL 512 //Total interrupts set
#define MYSTOS_PIT_FREQ 100 //PIT frequency, Hz
#define KERNEL_CODE_SEGMENT 0x08 //Code segment
#define KERNEL_DATA_SEGMENT 0x10 //Data segment
#define MYSTOS_HEAP_TOTAL_SIZE 104857600 //100 MB heap
#define MYSTOS_HEAP_BLOCK_SIZE 4096 //Do not change (paging aligned)
#define MYSTOS_HEAP_ADDRESS 0x01000000 //FFU RAM Extended memory
#define MYSTOS_HEAP_TABLE_ADDRESS 0x00007E00 //Real mode 480,5 KiB of usable conventional memory

#define MYSTOS_SECTOR_SIZE 512
#define MYSTOS_MAX_PATH_LEN 256
#define MYSTOS_MAX_FS 12
#define MYSTOS_MAX_DESCRIPTORS 512
#define MYSTOS_MAX_PATH 108

#define MYSTOS_TOTAL_GDT_SEGMENTS 3

#define sti 1
#define cli 0

#define FS_CHECK_FILE_CONTENTS "https://bit.ly/33Q1Aix"
#define FS_CHECK_FILE_SIZE 29

#define LOG_OK 0
#define LOG_ERR 1
#define LOG_WARN 2
#define LOG_NOTICE 3

//Colors
#define vga_black 0
#define vga_blue 1
#define vga_green 2
#define vga_cyan 3
#define vga_red 4
#define vga_magenta 5
#define vga_brown 6
#define vga_lightgray 7
#define vga_darkgray 8
#define vga_lightblue 9
#define vga_lightgreen 10
#define vga_lightcyan 11
#define vga_lightred 12
#define vga_lightmagenta 13
#define vga_yellow 14
#define vga_white 15

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

#endif