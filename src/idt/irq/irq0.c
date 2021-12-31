#include "irq.h"
#include "io/vgaio/vgaio.h"

#define PIT_A 0x40
#define PIT_CONTROL 0x43

#define PIT_RV 1193180
#define PIT_SET 0x36

#define SUBTICKS_PER_TICK 100

/*
 * Set the phase (in hertz) for the Programmable
 * Interrupt Timer (PIT).
 */
void timer_init(int hz){
	int divisor = PIT_RV / hz;
	outb(PIT_CONTROL, PIT_SET);
	outb(PIT_A, divisor & 0xFF);
	outb(PIT_A, (divisor >> 8) & 0xFF);
}

/*
 * Internal timer counters
 */
unsigned long timer_ticks = 0;
unsigned char timer_subticks = 0;

/*
 * IRQ handler for when the timer fires
 */
void IRQ0(){
	if (++timer_subticks == SUBTICKS_PER_TICK) {
		timer_ticks++;
		timer_subticks = 0;
	}
	//print(1, "tick,");
	//IRQ_ack(0);

    /*
     * MULTIPROCESSING
     */
	//wakeup_sleepers(timer_ticks, timer_subticks);
	//switch_task(1);
}

void relative_time(unsigned long seconds, unsigned long subseconds, unsigned long * out_seconds, unsigned long * out_subseconds){
	if (subseconds + timer_subticks > SUBTICKS_PER_TICK) {
		*out_seconds    = timer_ticks + seconds + 1;
		*out_subseconds = (subseconds + timer_subticks) - SUBTICKS_PER_TICK;
	} else {
		*out_seconds    = timer_ticks + seconds;
		*out_subseconds = timer_subticks + subseconds;
	}
}