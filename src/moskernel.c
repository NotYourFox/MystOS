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

void panic(const char* msg){
    char* tmp = (char*)msg;
    print("\nKernel panic - ");
    print(tolower(tmp));
    if (find(tmp, "!") < 0){
        print("!");
    }
    print(" MystOS halted.");
    while (1) {}
}

void kernel_main(){
    clear(); // Clear screen
    print("Starting MystOS...\n");
    kheap_init(); //Initialize the heap
    fs_init(); //Initialize the filesystems
    disks_init(); //Initialize the disks
    idt_init(); //Initialize the IDT

    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_PUBLIC_ACCESS); //Defining the page table
    page_switch(paging_4gb_chunk_get_dir(kernel_chunk));
    enable_paging();

    interrupt_flag(sti); //Allow interrupts

    int fd = fopen("0:/file.txt", "r");
    if (fd){
        printc("[+] ", 2);
        print("Filesystem is present\n");
    } else {
        panic("Filesystem is not present!");
    }
    printc("[+] ", 2);
    print("Done!\n");
}