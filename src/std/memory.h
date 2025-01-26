// kernel/memory.h (updated)
#pragma once

#include <stddef.h>
#include <stdint.h>

// Ensure proper alignment (16 bytes for general use)
#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

// Kernel memory functions
void* kmalloc(size_t size);
void kfree(void* ptr);