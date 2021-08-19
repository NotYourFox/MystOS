#include "io/io.h"
#include "disk.h"
#include "mem/mem.h"
#include "config.h"
#include "status.h"

struct disk disk;

int read_sector(int lba, int num, void* buf){
    outb(0x1F6, (lba >> 24) | 0xE0); //Highest 8 bytes of the LBA to the master drive
    outb(0x1F2, num);
    outb(0x1F3, (unsigned char)(lba & 0xFF)); //Lowest 8 bits
    outb(0x1F4, (unsigned char)(lba >> 8)); //Next 8 (previous 8 shifted out)
    outb(0x1F5, (unsigned char)(lba >> 16)); //Next 8 (previous 16 shifted out)
    outb(0x1F7, 0x20); //0x20 - Read Sectors With Retry

    unsigned short* ptr = (unsigned short*) buf;
    for (int i = 0; i < num; i++){
        //Waiting for the ATA to be ready
        char status = insb(0x1F7);
        while (!(status & 0x08)){ //0x08 equals 1000b, so we are checking the 4th bit for 1.
            status = insb(0x1F7);
        }
        //Copy from hard disk to memory
        for(int i = 0; i < 256; i++){
            *ptr = insw(0x1F0); //Reading 2 bytes into the buffer (256 times - 1 sector (512 bytes))
            ptr++;
        }
    }
    return 0;
}

void disks_init(){
    memset(&disk, 0, sizeof(disk)); //Sets the required amount of memory with zeros
    disk.type = MYSTOS_DT_REAL;
    disk.sector_size = MYSTOS_SECTOR_SIZE;
}

struct disk* get_disk(int index){ //This is just a test prototype of a disk abstraction
    if (index != 0){
        return 0;
    } 
    return &disk;
}

int read_block(struct disk* idisk, unsigned int lba, int num, void* buf){ //Read disk block
    if (idisk != &disk){
        return -EIO;
    }
    return read_sector(lba, num, buf);
}