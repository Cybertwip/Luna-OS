// kernel_stdlib.h
#pragma once

#include <stddef.h>

#define RAND_MAX 32767

int rand(void);
void srand(unsigned int seed);


// Kernel-specific memory allocation (no user-space malloc!)
void* kmalloc(size_t size);
void kfree(void* ptr);


#include_next <stdlib.h>