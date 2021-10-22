#ifndef VFS_H
#define VFS_H

#include "pparser.h"
#include <stdint.h>

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

enum{
    FILE_STAT_READ_ONLY = 0b00000001
};

typedef unsigned int FILE_STAT_FLAGS;

struct disk;

struct file_stat{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
};

typedef void* (*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int (*FS_READ_FUNCTION)(struct disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);
typedef int (*FS_CLOSE_FUNCTION)(void* private);
typedef int (*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode);
typedef int (*FS_STAT_FUNCTION)(struct disk* disk, void* private, struct file_stat* stat);

struct filesystem{
    FS_RESOLVE_FUNCTION resolve; //Return 0 if the disk is using supported filesystem
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;
    FS_CLOSE_FUNCTION close;
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
int fread(int desc_id, uint32_t size, uint32_t nmemb, void* ptr);
int fseek(int desc_id, int offset, FILE_SEEK_MODE whence);
int fstat(int desc_id, struct file_stat* stat);
int fclose(int desc_id);
void insert_fs(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);
#endif