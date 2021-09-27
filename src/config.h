#ifndef CONFIG_H
#define CONFIG_H

#define MYSTOS_INT_TOTAL 512 //Total interrupts set
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

#define sti 1
#define cli 0

#endif