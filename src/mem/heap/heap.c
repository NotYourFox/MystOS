#include "heap.h"
#include "status.h"
#include "moskernel.h"
#include "mem/mem.h"

static int heap_validate_table (void* ptr, void* end, struct heap_table* table){
    int res = 0;

    size_t table_size = (size_t)(end - ptr);
    size_t total_blocks = table_size / MYSTOS_HEAP_BLOCK_SIZE;
    if (table -> total != total_blocks){
        res = -EINVARG; //For someone, who dare changing the configs, or who accidentally caused an overflow
        goto out;
    }

out:
    return res;
}

static int heap_validate_alignment (void* ptr){
    return ((unsigned int)ptr % MYSTOS_HEAP_BLOCK_SIZE) == 0;
}

int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table){
    int res = 0;

    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end)){
        res = -EINVARG; //Guys, do not change the kernel configs, ok?
        goto out;
    }

    memset(heap, 0, sizeof(struct heap)); //Initialize the whole heap to 0
    heap -> saddr = ptr; //Heap start address
    heap -> table = table; //Heap table address
    res = heap_validate_table(ptr, end, table); //Check for the unexperienced man in the configs

    if (res < 0) {
        goto out;
    }

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table -> total; //Table size in bytes
    memset(table -> entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size); //Mark the whole table as free

out:
    return res;
}

static uint32_t heap_align_value_to_upper (uint32_t val){ //Align the value provided in kmalloc to the upper block size (e.g. 100 to 4096, 5000 to 8192, and so on)
    if ((val % MYSTOS_HEAP_BLOCK_SIZE) == 0){
        return val; //If already aligned, return
    }
    val = (val - (val % MYSTOS_HEAP_BLOCK_SIZE)); //We are taking the remainder of division by heap block size, and substracting it from the given value (5000 - 904 = 4096)
    val += MYSTOS_HEAP_BLOCK_SIZE; //We aligned the kmalloc value to the lower block, so just add 1 block to it,
    return val;
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry){
    return entry & 0xF; //Free or taken, ignoring the other types
}

int heap_get_start_block(struct heap* heap, uint32_t total_blocks){ //Finding the start block of the required memory to allocate
    struct heap_table* table = heap -> table;
    int block_counter = 0;
    int start_block = -1;

    for (size_t i = 0; i < table -> total; i++){
        if (heap_get_entry_type(table -> entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE){ //If not free, then count again
            block_counter = 0; //Count again
            start_block = -1;
            continue; //Loop
        }
        //If the entry is free
        //If it is the first block of free memory, then set the start block here
        if (start_block == -1) {
            start_block = i;
        }
        block_counter++; //Increment the block counter.
        if (block_counter == total_blocks){ //If the free block counter is equal to the number of the required blocks, then we found the free memory space to allocate
            break;
        }
        //Else continue.
    }

    if (start_block == -1){
        return -ENOMEM; //start_block will be equal -1 if the loop finishes, but the memory won't be found. It means, that we couldn't find enough memory in the heap.
    }

    return start_block;
}

void* heap_block_to_address(struct heap* heap, int block){
    return heap -> saddr + (block * MYSTOS_HEAP_BLOCK_SIZE); //Get the block address by it's number
}

void heap_mark_blocks_taken(struct heap* heap, int start_block, int total_blocks){
    int end_block = (start_block + total_blocks) - 1; //Calculate the end block
    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST; //It's the first block to mark. So, we are setting flags TAKEN and FIRST.
    if (total_blocks > 1){
        entry |= HEAP_BLOCK_HAS_NEXT; //If we allocate more, than one block, then we have next block to mark as taken (HAS_NEXT flag)
    }

    for(int i = start_block; i <= end_block; i++){ //From start to end...
        heap -> table -> entries[i] = entry; //Set the heap block entry
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN; //Since the next block is not first, we set only the TAKEN flag.
        if (i != end_block - 1){
            entry |= HEAP_BLOCK_HAS_NEXT; //But if it is not an end block (we are saying end_block - 1 for the end block, because we will set it on a next loop), it has a next block
        }
    }
}

void heap_mark_blocks_free(struct heap* heap, int start_block){ //We won't know the number of blocks to free, we just need to free the memory allocated on the provided address
    struct heap_table* table = heap -> table;
    for (int i = start_block; i < (int)table -> total; i++){ //So, we are trying to free all heap blocks from the start address
        HEAP_BLOCK_TABLE_ENTRY entry = table -> entries[i];
        table -> entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE; //Freeing
        if (!(entry & HEAP_BLOCK_HAS_NEXT)){ //But we mustn't free all the heap blocks, so we are actually checking if the block is the allocation end block, and breaking here. (We previously freed that block.)
            break;
        } 
    }
}

void* heap_malloc_blocks(struct heap* heap, int total_blocks){ //Allocate by BLOCKS
    void* address = 0;
    int start_block = heap_get_start_block(heap, total_blocks); //Get the start block
    if (start_block < 0){
        goto out; //If the start block is below zero (The ENOMEM exception, that can be returned by the function), we return, so the error code will be passed to the exception handler
    }
    address = heap_block_to_address(heap, start_block); //Getting the block address
    heap_mark_blocks_taken(heap, start_block, total_blocks); //Mark the blocks as taken
out:
    return address; //Return the allocated memory address
}

int heap_address_to_block(struct heap* heap, void* address){
    return ((int)(address - heap -> saddr) / MYSTOS_HEAP_BLOCK_SIZE); //Getting the block num by it's address
}

void* heap_malloc(struct heap* heap, size_t size){ //Allocate the number of bytes provided (Aligned to the upper block!)
    size_t aligned_size = heap_align_value_to_upper(size); //Align the value to the upper block (Memory fragmentation is possible!)
    uint32_t total_blocks = aligned_size / MYSTOS_HEAP_BLOCK_SIZE; //We get the total number of blocks to allocate...
    return heap_malloc_blocks(heap, total_blocks); //...and allocate them
}

void heap_free(struct heap* heap, void* ptr){ //Free the allocated blocks by pointer
    heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr)); //Get the block id, and free the memory by it.
}