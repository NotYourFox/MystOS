#include "irq.h"
#include "io/vgaio/vgaio.h"
#include "config.h"
#include "idt/idt.h"

void setIRQ(int irq, void* address){
    int saddr;
    if (irq < 0){
        return;
    } else if (irq < 8){
        saddr = 0x20 + irq;
    } else if (irq < 15){
        saddr = 0x82 + (irq - 8);
    } else {
        return;
    }
    idt_set(saddr, address);
}

void IRQ0_init(){
    log(1, LOG_NOTICE, "Initializing IRQ handler for vector 0x20 (IRQ 0)...");
    //setIRQ(0, IRQ0);
    timer_init(MYSTOS_PIT_FREQ);
    log(1, LOG_OK, "IRQ handler initialization complete.");
}

void IRQs_init(){
    IRQ0_init();
}

void IRQ_ack(int irq_no) {
	if (irq_no >= 12) {
		outb(0xA0, 0x20);
	}
	outb(0x20, 0x20);
}
