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

void boot_log(){
    int check = fopen("0:/file.txt", "r");
    if (check){
        log(1, LOG_OK, "Filesystem is present");
        char buf[strlen(FS_CHECK_FILE_CONTENTS)];
        log(1, LOG_NOTICE, "Testing filesystem R/W...");
        fseek(check, 6, SEEK_SET);
        fread(check, strlen(FS_CHECK_FILE_CONTENTS), 1, buf);
        struct file_stat stat;
        fstat(check, &stat);
        if (!strncmp(buf, FS_CHECK_FILE_CONTENTS, strlen(buf))){
            log(1, LOG_OK, "Seek mode - OK");
            log(1, LOG_OK, "Read mode - OK");
        } else {
            panic("R/W test failed!");
        }
        if (stat.filesize == FS_CHECK_FILE_SIZE){
            log(1, LOG_OK, "Stat mode - OK");
        } else {
            panic("R/W test failed!");
        }
        fclose(check);
        log(1, LOG_OK, "Close - OK");
    } else {
        panic("Filesystem is not present!");
    }
    print(1, "\n");
    struct rtc* rtc = time();
    print(6, inttostr(rtc -> day), ".", inttostr(rtc -> month), ".", inttostr(rtc -> year), "\n");
    print(6, inttostr(rtc -> hour), ":", inttostr(rtc -> minute), ":", inttostr(rtc -> second), "\n");
    print(1, "\n");
    //print(2, cpuinfo(), "\n");
    log(1, LOG_OK, "Done!");
}

struct gdt gdt_real[MYSTOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[MYSTOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00}, //NULL Segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9A}, //Kernel Code Segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92} //Kernel Data Segment
};

void kernel_main(){
    clear(); // Clear screen
    print(1, "Starting MystOS...\n");

    //Load the GDT
    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, MYSTOS_TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real));

    kheap_init(); //Initialize the heap
    fs_init(); //Initialize the filesystems
    disks_init(); //Initialize the disks
    idt_init(); //Initialize the IDT

    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_PUBLIC_ACCESS); //Defining the page table
    page_switch(paging_4gb_chunk_get_dir(kernel_chunk));
    enable_paging();

    interrupt_flag(sti); //Allow interrupts

    boot_log();
    printc(1, vga_yellow, "Happy New Year 2022!");
}
