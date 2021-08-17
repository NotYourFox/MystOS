#include "pagefile.h"
#include "mem/heap/kheap.h"
#include "mem/status.h"

void paging_load_dir(uint32_t* page_dir);

static uint32_t* current_dir = 0;

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags){
    uint32_t* page_dir = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++){
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++){
            entry[j] = (offset + (j * PAGING_PAGE_SIZE)) | flags;
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        page_dir[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE;
    }
    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb -> dir_entry = page_dir;
    return chunk_4gb;
}

void page_switch(uint32_t* page_dir){
    paging_load_dir(page_dir);
    current_dir = page_dir;
}

uint32_t* paging_4gb_chunk_get_dir(struct paging_4gb_chunk* chunk){
    return chunk -> dir_entry;
}

int paging_is_aligned(void* addr){
    return (uint32_t)addr % PAGING_PAGE_SIZE == 0;
}

int paging_get_indexes(void* virtual_addr, uint32_t* dir_index_out, uint32_t* table_index_out){
    int res = 0;
    if (!paging_is_aligned(virtual_addr)){
        res = -EINVARG;
        goto out;
    }

    *dir_index_out = ((uint32_t)virtual_addr / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t)virtual_addr % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);

out:
    return res;

}

int paging_set(uint32_t* page_dir, void* vaddr, uint32_t val){
    if (!paging_is_aligned(vaddr)){
        return -EINVARG;
    }

    uint32_t dir_index = 0;
    uint32_t table_index = 0;
    int res = paging_get_indexes(vaddr, &dir_index, &table_index);
    if (res < 0){
        return res;
    }

    uint32_t entry = page_dir[dir_index];
    uint32_t* table = (uint32_t*)(entry & 0xFFFFF00);
    table[table_index] = val;

    return 0;
}