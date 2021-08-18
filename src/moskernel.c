#include "moskernel.h"
#include "idt/idt.h"
#include "mem/heap/kheap.h"
#include "config.h"
#include "mem/page/pagefile.h"
#include "io/vgaio/vgaprint.h"

static struct paging_4gb_chunk* kernel_chunk = 0;

void kernel_main(){
    clear(); // clear screen
    print("Starting MystOS...\n", 15);
    kheap_init();
    idt_init();

    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_PUBLIC_ACCESS);
    page_switch(paging_4gb_chunk_get_dir(kernel_chunk));
    char* ptr = kzalloc(4096);
    paging_set(paging_4gb_chunk_get_dir(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_PUBLIC_ACCESS | PAGING_IS_WRITEABLE | PAGING_IS_PRESENT);
    enable_paging();

    interrupt_flag(sti);
}