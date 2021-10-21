#ifndef FS_H
#define FS_H

#include "pparser.h"

typedef unsigned int FILE_SEEK_MODE;
enum{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;
enum{
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

struct disk;
typedef void* (*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);

struct filesystem{
    FS_RESOLVE_FUNCTION resolve; //Return 0 is the disk is using supported filesystem
    FS_OPEN_FUNCTION open;

    char name[20];
};

struct file_desc{
    int index; //The descriptor index
    struct filesystem* filesystem;

    void* private; //Private data for internal file descriptor

    struct disk* disk; //The disk that the file descriptor should be used on
};

void fs_init();
int fopen(const char* filename, const char* mode_str);
void insert_fs(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);
#endif