#ifndef DISK_H
#define DISK_H

typedef unsigned int DISK_TYPE;

#define MYSTOS_DT_REAL 0 //Represents a physical hard disk

struct disk{
    DISK_TYPE type;
    int sector_size;
};

struct disk* get_disk(int index);
void disks_init();
int read_block(struct disk* idisk, unsigned int lba, int num, void* buf);

#endif