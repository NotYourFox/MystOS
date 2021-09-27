#include "moskernel.h"
#include "idt/idt.h"
#include "mem/heap/kheap.h"
#include "config.h"
#include "mem/page/pagefile.h"
#include "io/vgaio/vgaio.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "fs/fs.h"

static struct paging_4gb_chunk* kernel_chunk = 0;

void kernel_main(){
    clear(); // Clear screen
    print("Starting MystOS...");
    kheap_init(); //Initialize the heap
    fs_init(); //Initialize the filesystems
    disks_init(); //Initialize the disks
    idt_init(); //Initialize the IDT

    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_PUBLIC_ACCESS); //Defining the page table
    page_switch(paging_4gb_chunk_get_dir(kernel_chunk));
    enable_paging();

    interrupt_flag(sti); //Allow interrupts
    
    print("[+] Done!\n");
}