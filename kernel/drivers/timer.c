#include "sys/system.h"
#include "timer.h"
#include "arch/i386/descriptor_tables.h"
#include "screen.h"
#include "sys/scheduler.h"

// Static variable to keep track of ticks
static uint32_t tick = 0;

/**
 * @brief Timer interrupt callback function.
 * 
 * This function is called whenever the timer interrupt occurs. It increments
 * the tick count and prints the current tick value to the screen.
 * 
 * @param regs The CPU register state at the time of the interrupt (not used here).
 */
static void timer_callback(registers_t *regs) {
	tick++;
  schedule ();
}

uint32_t ticks() {
  return tick;
}

/**
 * @brief Initialize the Programmable Interval Timer (PIT).
 * 
 * This function sets up the PIT to generate timer interrupts at the specified
 * frequency. It also registers the timer callback function to handle the timer
 * interrupts.
 * 
 * @param freq The desired frequency in Hz for the timer.
 */
void init_timer(uint32_t freq) {
    uint32_t div;
    uint8_t lo, hi;

    // Register the timer callback to handle IRQ0.
    register_interrupt_handler(IRQ0, &timer_callback);

    /*
     * Calculate the divisor to achieve the desired frequency.
     * The PIT operates at a base frequency of 1193180 Hz. The divisor is used
     * to divide this base frequency to generate the desired frequency.
     */
    uint32_t divisor = 1193180 / freq;

    // Send the command byte to the PIT (0x36 = binary mode, mode 3, access mode: low/high bytes).
    outb(0x43, 0x36);

    // Split the divisor into low and high bytes.
    uint8_t l = (uint8_t)(divisor & 0xFF);
  	uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

    // Send the frequency divisor to the PIT data port.
    outb(0x40, l);
  	outb(0x40, h);
}

void udelay(uint32_t microseconds) {
    // Example: Busy-wait loop (adjust based on your CPU speed)
    for (uint32_t i = 0; i < microseconds * 1000; i++) {
        __asm__ volatile ("nop");
    }
}

void mdelay(uint32_t milliseconds) {
    while (milliseconds--) {
        udelay(1000); // 1000 microseconds = 1 millisecond
    }
}
