#ifndef IDT_H
#define IDT_H

#include <stdint.h>

//https://wiki.osdev.org/Interrupt_Descriptor_Table

struct int_desc {
    uint16_t offset_1; //offset bits 0 - 15
    uint16_t selector; // a code segment selector in GDT
    uint8_t zero;      // unused, set to 0
    uint8_t type_attr; // type and attributes
    uint16_t offset_2; // offset bits 16 - 31
} __attribute__((packed));

struct idtr_desc {
    uint16_t limit; //Limit - Maximum addressable byte in table
    uint32_t base; //Offset - Linear (paged) base address of IDT
} __attribute__((packed));

void idt_init();
void interrupt_flag(int flag);
void idt_set(int inum, void* address);

#endif