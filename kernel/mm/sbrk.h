#ifndef _KERNEL_SBRK_H
#define _KERNEL_SBRK_H

#include <stdint.h>

// Initialize the kernel's break position
void sbrk_init(void *start);

// Adjust the kernel's program break
void *sbrk(intptr_t increment);

#endif // _KERNEL_SBRK_H