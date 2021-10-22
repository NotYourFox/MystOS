#include "moskernel.h"
#include "idt/idt.h"
#include "mem/heap/kheap.h"
#include "config.h"
#include "mem/page/pagefile.h"
#include "io/vgaio/vgaio.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "fs/vfs.h"

static struct paging_4gb_chunk* kernel_chunk = 0;

void panic(const char* msg){
    char* tmp = (char*)msg;
    if(!is_linefeed()){
        print("\n");
    }
    printc("[-] ", vga_red);
    print("Kernel panic - ");
    print(tolower(tmp));
    if (find(tmp, "!") < 0){
        print("!");
    }
    print(" MystOS halted.");
    interrupt_flag(cli);
    while (1) {}
}

void log(const char* msg, int res){
    switch(res){
        case LOG_OK:
            printc("[+] ", vga_green);
            print(strcat(msg, "\n"));
            break;
        case LOG_ERR:
            printc("[-] ", vga_red);
            print(strcat(msg, "\n"));
            break;
        case LOG_WARN:
            printc("[!] ", vga_yellow);
            print(strcat(msg, "\n"));
            break;
        case LOG_CHECK:
            printc("[=] ", vga_lightblue);
            print(strcat(msg, "\n"));
            break;
    }
}

void boot_log(){
    int check = fopen("0:/file.txt", "r");
    if (check){
        log("Filesystem is present", LOG_OK);
        char buf[strlen(FS_CHECK_FILE_CONTENTS)];
        log("Testing filesystem R/W...", LOG_CHECK);
        fseek(check, 6, SEEK_SET);
        fread(check, strlen(FS_CHECK_FILE_CONTENTS), 1, buf);
        struct file_stat stat;
        fstat(check, &stat);
        if (!strncmp(buf, FS_CHECK_FILE_CONTENTS, strlen(buf))){
            log("Seek mode - OK", LOG_OK);
            log("Read mode - OK", LOG_OK);
        } else {
            panic("R/W test failed!");
        }
        if (stat.filesize == FS_CHECK_FILE_SIZE){
            log("Stat mode - OK", LOG_OK);
        } else {
            panic("R/W test failed!");
        }
        fclose(check);
        log("Close - OK", LOG_OK);
    } else {
        panic("Filesystem is not present!");
    }
    print("\n");
    log("Done!", LOG_OK);
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

    boot_log();
}