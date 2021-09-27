#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "io/vgaio/vgaio.h"
#include "mem/mem.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init() {
    int total_table_entries = MYSTOS_HEAP_TOTAL_SIZE / MYSTOS_HEAP_BLOCK_SIZE; //Total heap blocks
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(MYSTOS_HEAP_TABLE_ADDRESS);
    kernel_heap_table.total = total_table_entries;

    void* end = (void*)(MYSTOS_HEAP_ADDRESS + MYSTOS_HEAP_TOTAL_SIZE); //End of heap
    int res = heap_create(&kernel_heap, (void*)(MYSTOS_HEAP_ADDRESS), end, &kernel_heap_table); //Trying to create heap 
    if (res < 0) { //Handle the error, if there is any.
        panic("Failed to create heap!");
    }
}

void* kmalloc(size_t size){
    return heap_malloc(&kernel_heap, size);
}

void* kzalloc(size_t size){
    void* ptr = kmalloc(size);
    if (!ptr){
        return 0;
    }
    memset(ptr, 0, size);
    return ptr;
}

void kfree(void* ptr){
    heap_free(&kernel_heap, ptr);
}
