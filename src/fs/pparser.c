#include "pparser.h"
#include "config.h"
#include "mem/heap/kheap.h"
#include "mem/mem.h"
#include "io/vgaio/vgaio.h"
#include "status.h"
// Path example: /dev/dsk1/home/user/desktop/folder/file.txt
//                 ^   ^
//      device prefix  |
//                   disk 1
//
//               /dev2/dsk1/home/user2/...
//                 ^    ^
//           device 2   |
//   (in local network) |
//                  disk on PC
//             (in local network)
//

//:/
//:/

static int validate_path_format(const char* filename){ //flp1, flp2; dsk1, dsk2; cdd1, cdd2;
    int len = strnlen(filename, MYSTOS_MAX_PATH_LEN); // /dev/dsk1/home/username/desktop/folder/file.txt = ~/desktop/folder/file.txt
    return (len >= 3 && isdigit(filename[0]) && memcmp((void*)&filename[1], ":/", 2) == 0);
}

static int get_drive_by_path(const char** path){
    if (!validate_path_format(*path)){
        return -EBADPATH;
    }

    int drive_num = strtoint(*path[0]);
    *path += 3; //Add 3 bytes to skip the drive number
    return drive_num;
}

static struct path_root* create_root(int drive_num){
    struct path_root* path_r = kzalloc(sizeof(struct path_root));
    path_r -> drive_index = drive_num;
    path_r -> first = 0x00;
    return path_r;
}

static const char* get_path_part(const char** path){
    char* path_part = kzalloc(MYSTOS_MAX_PATH_LEN);
    int i = 0;
    while(**path != '/' && **path != 0x00){
        path_part[i] = **path;
        *path += 1;
        i++;
    }

    if (**path == '/'){
        *path += 1;
    }

    if (i == 0){
        kfree(path_part);
        path_part = 0;
    }

    return path_part;
}

struct path_part* parse_path_part(struct path_part* prev_part, const char** path){
    const char* path_part_str = get_path_part(path);
    if (!path_part_str){
        return 0;
    }

    struct path_part* part = kzalloc(sizeof(struct path_part));
    part -> part = path_part_str;
    part -> next = 0x00;

    if (prev_part){
        prev_part -> next = part;
    }

    return part;
}

void parser_free(struct path_root* root){
    struct path_part* part = root->first;
    while(part){
        struct path_part* next_part = part -> next;
        kfree((void*)part -> part);
        kfree(part);
        part = next_part;
    }

    kfree(root);
}

struct path_root* parse(const char* path, const char* current_dir){
    int res = 0;
    const char* tmp_path = path;
    struct path_root* path_root = 0;

    if (strlen(path) > MYSTOS_MAX_PATH_LEN){
        goto out;
    }

    res = get_drive_by_path(&tmp_path);
    if (res < 0){
        goto out;
    }

    path_root = create_root(res);
    if (!path_root){
        goto out;
    }

    struct path_part* first_part = parse_path_part(NULL, &tmp_path);
    if (!first_part){
        goto out;
    }

    path_root -> first = first_part;
    struct path_part* part = parse_path_part(first_part, &tmp_path);
    while(part){
        part = parse_path_part(part, &tmp_path);
    }

out:
    return path_root;
}