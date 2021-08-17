#ifndef PAGEFILE_H
#define PAGEFILE_H

#include <stdint.h>
#include <stddef.h>

#define PAGING_CACHE_DISABLED 0b00010000
#define PAGING_WRITE_THROUGH 0b00001000
#define PAGING_PUBLIC_ACCESS 0b00000100
#define PAGING_IS_WRITEABLE 0b00000010
#define PAGING_IS_PRESENT 0b00000001

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

struct paging_4gb_chunk {
    uint32_t* dir_entry;
};

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags);
void page_switch(uint32_t* page_dir);
void enable_paging();
uint32_t* paging_4gb_chunk_get_dir(struct paging_4gb_chunk* chunk);
int paging_set(uint32_t* page_dir, void* vaddr, uint32_t val);
int paging_is_aligned(void* addr);

#endif