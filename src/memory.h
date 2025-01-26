// kernel/memory.h
#pragma once

#include <stddef.h>
#include <stdint.h> // for size_t

// Kernel memory allocation and deallocation functions
void* kmalloc(size_t size);
void kfree(void* ptr);
