#include "pagefile.h"
#include "mem/heap/kheap.h"
#include "status.h"

//What on earth is this hell? I cannot comment it! Pls help

void paging_load_dir(uint32_t* page_dir);

static uint32_t* current_dir = 0;

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags){
    uint32_t* page_dir = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE); //Kzalloc the required amount of bytes (each entry is uint32_t)
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++){
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE); //Kzalloc each page table entry, so all tables are defined by zero
        for (int j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++){
            entry[j] = (offset + (j * PAGING_PAGE_SIZE)) | flags; //Define each page table with it's own virtual address range (1st will start at 0, second at 4096, and so on)
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE); //New directory (so new page tables will point to required address but with an page dir offset)
        page_dir[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE; //Defining the directory with page tables (set it to writeable)
    }
    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb -> dir_entry = page_dir; //chunk_4gb will contain our page directory
    return chunk_4gb; //Returning the directory we defined
}

void page_switch(uint32_t* page_dir){
    paging_load_dir(page_dir);
    current_dir = page_dir;
}

uint32_t* paging_4gb_chunk_get_dir(struct paging_4gb_chunk* chunk){
    return chunk -> dir_entry;
}

int paging_is_aligned(void* addr){
    return (uint32_t)addr % PAGING_PAGE_SIZE == 0; //Check the paging alignment (for someone, who likes changing configs, obviously)
}

int paging_get_indexes(void* virtual_addr, uint32_t* dir_index_out, uint32_t* table_index_out){
    int res = 0;
    if (!paging_is_aligned(virtual_addr)){
        res = -EINVARG; //Yeah, seriously.
        goto out;
    }

    *dir_index_out = ((uint32_t)virtual_addr / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE)); //Getting the directory number by virtual address
    *table_index_out = ((uint32_t)virtual_addr % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE); //Getting the table number by virtual address

out:
    return res;

}

int paging_set(uint32_t* page_dir, void* vaddr, uint32_t val){
    if (!paging_is_aligned(vaddr)){
        return -EINVARG; //And again. I see you!
    }

    uint32_t dir_index = 0;
    uint32_t table_index = 0;
    int res = paging_get_indexes(vaddr, &dir_index, &table_index); //Getting dir and table indexes
    if (res < 0){
        return res;
    }

    uint32_t entry = page_dir[dir_index]; //Getting a pointer to a page directory
    uint32_t* table = (uint32_t*)(entry & 0xFFFFF000); //We don't want the flags, so they get truncated. We are getting a pointer to the tables.
    table[table_index] = val; //Assigning the real memory address to the table

    return 0;
}