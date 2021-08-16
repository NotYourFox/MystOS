#include "idt.h"
#include "hskernel.h"
#include "config.h"
#include "mem/mem.h"
#include "io/io.h"

struct int_desc idt_descriptors[HELIOS_INT_TOTAL]; //IDT descriptors array
struct idtr_desc idtr_descriptor; //IDTR descriptor
extern void idt_load(struct idtr_desc* ptr);
extern void INT21H();
extern void no_int();

void INT21H_HANDLER(){
    print("Keyboard interrupt!\n", 15);
    outb(0x20, 0x20);
}

void no_int_handler(){
    outb(0x20, 0x20);
}

void idt_set(int inum, void* address){
    struct int_desc* desc = &idt_descriptors[inum];
    desc -> offset_1 = (uint32_t) address & 0xFFFF;
    desc -> selector = KERNEL_CODE_SEGMENT;
    desc -> zero = 0x00;
    desc -> type_attr = 0xEE;
    desc -> offset_2 = (uint32_t) address >> 16;
}

void idt_init(){
    memset(idt_descriptors, 0, sizeof(idt_descriptors)); //Fill the required memory with zeros
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1; //Limit - Maximum addressable byte in table
    idtr_descriptor.base = (uint32_t) idt_descriptors; //IDT address

    for (int i = 0; i < HELIOS_INT_TOTAL; i++){
        idt_set(i, no_int);
    }

    idt_set(0x21, INT21H);
    idt_load(&idtr_descriptor);
}

