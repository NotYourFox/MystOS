#ifndef DISKSTREAM_H
#define DISKSTREAM_H

#include "disk.h"

struct disk_stream{
    int pos;
    struct disk* disk;
};

struct disk_stream* new_stream(int disk_id);
int stream_seek(struct disk_stream* stream, int pos);
int stream_read(struct disk_stream* stream, void* out, int num);
void stream_close(struct disk_stream* stream);

#endif