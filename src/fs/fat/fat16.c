#include "fat16.h"
#include "status.h"
#include "io/vgaio/vgaio.h"
#include "disk/disk.h"
#include "disk/stream.h"
#include "mem/heap/kheap.h"
#include "mem/mem.h"
#include <stdint.h>

#define MYSTOS_FAT16_SIGNATURE 0x29
#define MYSTOS_FAT16_FAT_ENTRY_SIZE 0x02
#define MYSTOS_FAT16_BAD_SECTOR 0xFF7
#define MYSTOS_FAT16_UNUSED 0x00

typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

//FAT directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

struct fat_header_extended{
    uint8_t DriveNumber;
    uint8_t WinNTBit;
    uint8_t Signature;
    uint32_t VolumeID;
    uint8_t VolumeIDString[11];
    uint8_t SystemIDString[8];
} __attribute__((packed));

struct fat_header{
    uint8_t ShortJmp[3];
    uint8_t OEMIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FATCopies;
    uint16_t RootDirEntries;
    uint16_t NumberOfSectors;
    uint8_t MediaType;
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;
    uint16_t NumberOfHeads;
    uint32_t HiddenSectors;
    uint32_t SectorsBig;
} __attribute__((packed));

struct fat_h{
    struct fat_header primary_header;
    union fat_h_e
    {
        struct fat_header_extended extended_header;
    } shared;
};

struct fat_directory_item{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_time;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory{
    struct fat_directory_item* item;
    int total;
    int sector_pos;
    int ending_sector_pos;
};

struct fat_item{
    union{
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };

    FAT_ITEM_TYPE type;
};

struct fat_item_descriptor{
    struct fat_item* item;
    uint32_t pos;
};

struct fat_private{
    struct fat_h header;
    struct fat_directory root_directory;

    struct disk_stream* cluster_read_stream; //Stream the data clusters
    struct disk_stream* fat_read_stream; //Stream the FAT
    struct disk_stream* directory_stream; //Stream the directory
};

int fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);

struct filesystem fat16_fs = 
{
    .resolve = fat16_resolve,
    .open = fat16_open,
};

struct filesystem* fat16_init(){
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}

static void fat16_init_private(struct disk* disk, struct fat_private* private){
    memset(private, 0, sizeof(struct fat_private));
    private -> cluster_read_stream = new_stream(disk -> id);
    private -> fat_read_stream = new_stream(disk -> id);
    private -> directory_stream = new_stream(disk -> id);
}

int fat16_sector_to_absolute(struct disk* disk, int sector){
    return sector * disk -> sector_size;
}

int fat16_get_total_items_for_directory(struct disk* disk, uint32_t directory_start_sector){
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&empty_item, 0, sizeof(empty_item));

    struct fat_private* fat_private = disk -> fs_private;

    int res = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * disk -> sector_size;
    struct disk_stream* stream = fat_private -> directory_stream;
    if (stream_seek(stream, directory_start_pos) != 0){
        res = -EIO;
        goto out;
    }

    while (1){
        if (stream_read(stream, &item, sizeof(item)) != 0){
            res = -EIO;
            goto out;
        }

        if (item.filename[0] == 0x00){
            break; //Done
        }

        if (item.filename[0] == 0xE5){ //If the item is unused
            continue;
        }

        i++;
    }

    res = i;

out:
    return res;
}

int fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* directory){
    int res = 0;
    struct fat_header* primary_header = &fat_private -> header.primary_header;
    int root_dir_sector_pos = primary_header -> FATCopies * primary_header -> SectorsPerFAT + primary_header -> ReservedSectors;
    int root_dir_entries = fat_private -> header.primary_header.RootDirEntries;
    int root_dir_size = root_dir_entries * sizeof(struct fat_directory_item);
    int total_sectors = root_dir_size / disk -> sector_size;
    if (root_dir_size % disk -> sector_size){
        total_sectors += 1;
    }

    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);
    struct fat_directory_item* dir = kzalloc(root_dir_size);
    if (!dir){
        res = -ENOMEM;
        goto out;
    }

    struct disk_stream* stream = fat_private -> directory_stream;
    if (stream_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != 0){
        res = -EIO;
        goto out;
    }

    if (stream_read(stream, dir, root_dir_size) != 0){
        res = -EIO;
        goto out;
    }

    directory -> item = dir;
    directory -> total = total_items;
    directory -> sector_pos = root_dir_sector_pos;
    directory -> ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk -> sector_size);
out:
    return res;
} 
int fat16_resolve(struct disk* disk){
    int res = 0;
    struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));
    fat16_init_private(disk, fat_private);

    disk -> fs_private = fat_private;
    disk -> filesystem = &fat16_fs;
    
    struct disk_stream* stream = new_stream(disk -> id);
    if (!stream){
        res = -ENOMEM;
        goto out;
    }

    if (stream_read(stream, &fat_private -> header, sizeof(fat_private -> header)) != 0){
        res = -EIO;
        goto out;
    }

    if (fat_private->header.shared.extended_header.Signature != 0x29){
        res = -EOTHERFS;
    }

    if (fat16_get_root_directory(disk, fat_private, &fat_private -> root_directory) != 0){
        res = -EIO;
        goto out;
    }

out:
    if (stream){
        stream_close(stream);
    }

    if (res < 0){
        kfree(fat_private);
        disk -> fs_private = 0;
    }
    return res;
}

void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode){
    return 0;
}