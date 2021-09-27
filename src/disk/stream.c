#include "stream.h"
#include "mem/heap/kheap.h"
#include "config.h"

struct disk_stream* new_stream(int disk_id){
    struct disk* disk = get_disk(disk_id);
    if (!disk){
        return 0;
    }
    struct disk_stream* stream = kzalloc(sizeof(struct disk_stream));
    stream -> pos = 0;
    stream -> disk = disk;
    return stream;
}

int stream_seek(struct disk_stream* stream, int pos){
    stream -> pos = pos;
    return 0;
}

int stream_read(struct disk_stream* stream, void* out, int num){
    int sector = stream -> pos / MYSTOS_SECTOR_SIZE;
    int offset = stream -> pos % MYSTOS_SECTOR_SIZE;
    char buf[MYSTOS_SECTOR_SIZE];

    int res = read_block(stream -> disk, sector, 1, buf);
    if (res < 0){
        goto out;
    }
    int total_bytes_to_read = num >  MYSTOS_SECTOR_SIZE ? MYSTOS_SECTOR_SIZE : num;
    for (int i = 0; i < total_bytes_to_read; i++){
        *(char*)out++ = buf[offset + i];
    }
    stream -> pos += total_bytes_to_read; //Adjust the stream
    if (num > MYSTOS_SECTOR_SIZE){ //We can only read 1 sector at a time. So if we want to read more, we are going into recursion.
        res = stream_read(stream, out, num - MYSTOS_SECTOR_SIZE);
    }
out:
    return res;
}

void stream_close(struct disk_stream* stream){
    kfree(stream);
}