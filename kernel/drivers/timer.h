#ifdef __cplusplus
extern "C" {
#endif

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

void udelay(uint32_t microseconds);

void mdelay(uint32_t milliseconds);

#endif /* TIMER_H */

#ifdef __cplusplus
}
#endif