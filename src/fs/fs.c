#include "fs.h"
#include "config.h"
#include "mem/mem.h"
#include "status.h"
#include "mem/heap/kheap.h"
#include "io/vgaio/vgaio.h"
#include "fat/fat16.h"
#include "moskernel.h"

struct filesystem* filesystems[MYSTOS_MAX_FS];
struct file_desc* file_descriptors[MYSTOS_MAX_DESCRIPTORS];

static struct filesystem** get_free_fs(){
    int i = 0;
    for (i = 0; i < MYSTOS_MAX_FS; i++){
        if (filesystems[i] == 0){
            return &filesystems[i];
        }
    }

    return 0;
}

void insert_fs(struct filesystem* filesystem){
    struct filesystem** fs;
    fs = get_free_fs();
    if (!fs){
        panic("Could not insert filesystem!");
    }

    *fs = filesystem;
}

static void static_load(){
    insert_fs(fat16_init());
}

void fs_load(){
    memset(filesystems, 0, sizeof(filesystems));
    static_load();
}

void fs_init(){
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static int new_file_descriptor(struct file_desc** desc_out){
    int res = -ENOMEM;
    for (int i = 0; i < MYSTOS_MAX_DESCRIPTORS; i++){
        if (!file_descriptors[i]){
            struct file_desc* desc = kzalloc(sizeof(struct file_desc));
            //Descriptors should start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }

    return res;
}

static struct file_desc* get_file_descriptor(int fd){
    if (fd == 0 || fd >= MYSTOS_MAX_DESCRIPTORS){
        return 0;
    }

    int index = fd - 1; //Descriptors start at 1
    return file_descriptors[index];
}

struct filesystem* fs_resolve(struct disk* disk){
    struct filesystem* fs = 0;
    for (int i = 0; i < MYSTOS_MAX_FS; i++){
        if (filesystems[i] != 0 && filesystems[i] -> resolve(disk) == 0){
            fs = filesystems[i];
            break;
        }
    }
    
    return fs;
}
int fopen(const char* filename, const char* mode){
    return -EIO;
}