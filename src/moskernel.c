#include "moskernel.h"
#include "idt/idt.h"
#include "mem/heap/kheap.h"
#include "config.h"
#include "mem/page/pagefile.h"
#include "io/vgaio/vgaio.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "fs/vfs.h"
#include "cmos/cmos.h"
//#include "cpu/cpu.h"
#include "idt/irq/irq.h"
#include "gdt/gdt.h"
#include "mem/mem.h"
#include "task/tss.h"

static struct paging_4gb_chunk* kernel_chunk = 0;

void panic(const char* msg){
    char* tmp = (char*)msg;
    if(!is_linefeed()){
        print(1, "\n");
    }
    printc(1, vga_red, "[-] ");
    print(3, "Kernel panic - ", tolower(tmp), " MystOS halted.");
    interrupt_flag(cli);
    while (1) {}
}

void check_fs(){
    int check = fopen("0:/file.txt", "r");
    if (check){
        log(1, LOG_OK, "Filesystem is present.");
        char buf[strlen(FS_CHECK_FILE_CONTENTS)];
        log(1, LOG_NOTICE, "Testing filesystem R/W...");
        fseek(check, 6, SEEK_SET);
        fread(check, strlen(FS_CHECK_FILE_CONTENTS), 1, buf);
        struct file_stat stat;
        fstat(check, &stat);
        if ((strncmp(buf, FS_CHECK_FILE_CONTENTS, strlen(buf))) || (stat.filesize != FS_CHECK_FILE_SIZE)){
            panic("R/W test failed!");
        }
        fclose(check);
        log(1, LOG_OK, "R/W test complete.");
    } else {
        panic("Filesystem is not present!");
    }
}

struct tss tss;

struct gdt gdt_real[MYSTOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[MYSTOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00}, //NULL segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9A}, //Kernel dode segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92}, //Kernel data segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF8}, //User code segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF2}, //User data segment
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9} //TSS segment
};

void kernel_main(){
    clear(); // Clear screen
    print(1, "Starting MystOS...\n");
    log(1, LOG_NOTICE, "Initialising the system...");

    //Load the GDT
    log(1, LOG_NOTICE, "Attempting to reinitialise the Global Descriptor Table...");
    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, MYSTOS_TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real));

    log(1, LOG_NOTICE, "Initialising heap entries...");
    kheap_init(); //Initialize the heap
    log(1, LOG_NOTICE, "Initialising the filesystems...");
    fs_init(); //Initialize the filesystems
    log(1, LOG_NOTICE, "Initialising the disks...");
    disks_init(); //Initialize the disks
    check_fs();
    log(1, LOG_NOTICE, "Initialising the Interrupt Descriptor Table...");
    idt_init(); //Initialize the IDT

    //Setup and load the TSS
    log(1, LOG_NOTICE, "Setting up the Task Switch Segment...");
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SEGMENT;
    tss_load(0x28);
    log(1, LOG_OK, "TSS setup process complete.");

    log(1, LOG_NOTICE, "Initializing the kernel paging chunk (4 GB)...");
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_PUBLIC_ACCESS); //Defining the page table
    page_switch(paging_4gb_chunk_get_dir(kernel_chunk));
    log(1, LOG_NOTICE, "Attempting to enable paging...");
    enable_paging();

    interrupt_flag(sti); //Allow interrupts

    log(1, LOG_OK, "System initialisation complete.");
    //print(2, cpuinfo(), "\n");
    log(1, LOG_OK, "Done!");
    printc(1, vga_yellow, "Happy New Year 2022!");
}
