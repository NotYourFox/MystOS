#include "gdt.h"
#include "moskernel.h"
#include "io/vgaio/vgaio.h"
#include "config.h"

void load_gdt(struct gdt* gdt, int size);

void gdt_load(struct gdt* gdt, int size){
    load_gdt(gdt, size);
    log(1, LOG_OK, "GDT reinitialization process complete.");
}

void encodeGdtEntry(uint8_t* target, struct gdt_structured src){
    if ((src.limit > 65536) && ((src.limit & 0xFFF) != 0xFFF)){
        panic("encodeGdtEntry: Invalid argument.");
    }

    target[6] = 0x40;
    if (src.limit > 65536){
        src.limit = src.limit >> 12;
        target[6] = 0xC0;
    }

    //Encodes the limit
    target[0] = src.limit & 0xFF;
    target[1] = (src.limit >> 8) & 0xFF;
    target[6] |= (src.limit >> 16) & 0x0F;

    //Encodes the base
    target[2] = src.base & 0xFF;
    target[3] = (src.base >> 8) & 0xFF;
    target[4] = (src.base >> 16) & 0xFF;
    target[7] = (src.base >> 24) & 0xFF;

    //Sets the type
    target[5] = src.type;
}

void gdt_structured_to_gdt(struct gdt* gdt, struct gdt_structured* structured_gdt, int total_entries){
    for (int i = 0; i < total_entries; i++){
        encodeGdtEntry((uint8_t*)&gdt[i], structured_gdt[i]);
    }
}