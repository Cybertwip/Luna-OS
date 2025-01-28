#ifndef TIMER_H
#define TIMER_H

#include "sys/system.h"

/**
 * @brief Initialize the Programmable Interval Timer (PIT).
 * 
 * This function sets up the PIT to generate interrupts at the specified
 * frequency. These interrupts are used for system timing and scheduling.
 * 
 * @param freq The desired interrupt frequency in Hertz (Hz).
 */
void init_timer(uint32_t freq);

#endif /* TIMER_H */
