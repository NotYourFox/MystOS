#include "vfs.h"
#include "config.h"
#include "mem/mem.h"
#include "status.h"
#include "disk/disk.h"
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
    printc("[+] ", 2);
    print(strcat(strcat(strcat(strcat("Resolved ", fs -> name), " on "), inttostr(disk -> id)), ":/\n"));
    if (!fs){
        panic("Could not resolve filesystem!");
    }
    return fs;
}

FILE_MODE get_file_mode_by_string(const char* str){
    FILE_MODE mode = FILE_MODE_INVALID;
    if (strncmp(str, "r", 1) == 0){
        mode = FILE_MODE_READ;
    }
    else if (strncmp(str, "w", 1) == 0){
        mode = FILE_MODE_WRITE;
    }
    else if (strncmp(str, "a", 1) == 0){
        mode = FILE_MODE_APPEND;
    }
    return mode;
}

int fopen(const char* filename, const char* mode_str){
    int res = 0;
    struct path_root* root_path = parse(filename, NULL);
    if (!root_path){
        res = -EINVARG;
        goto out;
    }

    if (!root_path -> first){
        res = -EINVARG;
        goto out;
    }

    struct disk* disk = get_disk(root_path -> drive_index);
    if (!disk){
        res = -EIO;
        goto out;
    }

    if (!disk -> filesystem){
        res = -EIO;
        goto out;
    }

    FILE_MODE mode = get_file_mode_by_string(mode_str);
    if (mode == FILE_MODE_INVALID){
        res = -EINVARG;
        goto out;
    }

    void* descriptor_private_data = disk -> filesystem -> open(disk, root_path -> first, mode);
    if (is_error(descriptor_private_data)){
        res = error_int(descriptor_private_data);
        goto out;
    }

    struct file_desc* desc = 0;
    res = new_file_descriptor(&desc);
    if (res < 0){
        goto out;
    }
    desc -> filesystem = disk -> filesystem;
    desc -> private = descriptor_private_data;
    desc -> disk = disk;
    res = desc -> index;

out:
    //FOPEN shouldn't return a negative value
    if (res < 0){
        res = 0;
    }
    return res;
}

int fstat(int desc_id, struct file_stat* stat){
    int res = 0;
    struct file_desc* desc = get_file_descriptor(desc_id);
    if (!desc){
        res = -EIO;
        goto out;
    }
    res = desc -> filesystem -> stat(desc -> disk, desc -> private, stat);

out:
    return res;
}

int fseek(int desc_id, int offset, FILE_SEEK_MODE whence){
    int res = 0;
    struct file_desc* desc = get_file_descriptor(desc_id);
    if (!desc){
        res = -EIO;
        goto out;
    }
    res = desc -> filesystem -> seek(desc -> private, offset, whence);
out:
    return res;
}

int fread(int desc_id, uint32_t size, uint32_t nmemb, void* ptr){
    int res = 0;
    if (size <= 0 || nmemb <= 0 || desc_id < 1){
        res = -EINVARG;
        goto out;
    }

    struct file_desc* desc = get_file_descriptor(desc_id);
    if (!desc){
        res = -EINVARG;
        goto out;
    }

    res = desc -> filesystem -> read(desc -> disk, desc -> private, size, nmemb, (char*)ptr);
out:
    return res;
}

int fclose(int desc_id){
    int res = 0;
    struct file_desc* desc = get_file_descriptor(desc_id);
    if (!desc){
        res = -EIO;
        goto out;
    }

    res = desc -> filesystem -> close(desc -> private);
out:
    return res;
}