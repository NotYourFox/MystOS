#include "moskernel.h"
#include "idt/idt.h"
#include "mem/heap/kheap.h"
#include "config.h"
#include "mem/page/pagefile.h"
#include "io/vgaio/vgaprint.h"
#include "disk/disk.h"

static struct paging_4gb_chunk* kernel_chunk = 0;

void kernel_main(){
    clear(); // Clear screen
    print("Starting MystOS...\n");
    kheap_init(); //Initialize the heap
    disks_init();
    idt_init(); //Initialize the IDTm

    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_PUBLIC_ACCESS); //Defining the page table
    page_switch(paging_4gb_chunk_get_dir(kernel_chunk));
    enable_paging();

    interrupt_flag(sti); //Allow interrupts
    print("[+] Done!\n");
}