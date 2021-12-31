#include "idt.h"
#include "io/vgaio/vgaio.h"
#include "config.h"
#include "mem/mem.h"
#include "io/io.h"
#include "irq/irq.h"

//https://wiki.osdev.org/Interrupt_Descriptor_Table

struct int_desc idt_descriptors[MYSTOS_INT_TOTAL]; //IDT descriptors array
struct idtr_desc idtr_descriptor; //IDTR descriptor
extern void load_idt(struct idtr_desc* ptr);
extern void INT21H();
extern void no_int();

void idt_load(struct idtr_desc* ptr){
    load_idt(ptr);
    log(1, LOG_OK, "Succesfully loaded the IDT");
}

void INT21H_HANDLER(){
    printc(1, vga_lightblue, "Happy New Year 2022!\n"); //Handler for the 21h (keyboard) interrupt
    outb(0x20, 0x20);
}

void no_int_handler(){
    outb(0x20, 0x20); //Prevents the kernel panic
}

void idt_set(int inum, void* address){
    struct int_desc* desc = &idt_descriptors[inum];
    desc -> offset_1 = (uint32_t) address & 0xFFFF; //Offset bits 0-15 (16-31 bits truncated)
    desc -> selector = KERNEL_CODE_SEGMENT; //Code segment selector
    desc -> zero = 0x00; //Zero
    desc -> type_attr = 0xEE; //Type: 0xE (32-bit interrupt gate); Attr: 0xE (1110b)
    desc -> offset_2 = (uint32_t) address >> 16; //Offset bits 16-31 (0-15 bits truncated)
    if (address != no_int){
        log(2, LOG_OK, "Successfuly set the interrupt ", hex(inum));
    }
}

void idt_init(){
    memset(idt_descriptors, 0, sizeof(idt_descriptors)); //Fill the required memory with zeros
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1; //Limit - Maximum addressable byte in table
    idtr_descriptor.base = (uint32_t) idt_descriptors; //IDT address

    for (int i = 0; i < MYSTOS_INT_TOTAL; i++){
        idt_set(i, no_int); //Set no_int handler for undefined interrupts
        //log(2, LOG_WARN, "No interrupt handler for vector ", hex(i));
    }

    IRQs_init();
    idt_set(0x21, INT21H); //Assembly interrupt handler address (21h)
    idt_load(&idtr_descriptor); //Assembly function to load IDT (LIDT instruction). Accepts the address of table and a maximum addressable byte.
}

