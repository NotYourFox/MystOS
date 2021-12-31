#ifndef IRQ_H
#define IRQ_H

#include "io/io.h"

//IRQ 0
void IRQs_init();
void IRQ_ack(int irq_no);
void timer_init(int hz);
void IRQ0();
void relative_time(unsigned long seconds, unsigned long subseconds, unsigned long * out_seconds, unsigned long * out_subseconds);

#endif