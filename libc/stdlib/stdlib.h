// kernel_stdlib.h
#pragma once

#include <stddef.h>
#include <sys/types.h>

#include <pdclib/_PDCLIB_glue.h>

#define RAND_MAX 32767

int rand(void);
void srand(unsigned int seed);


// Kernel-specific memory allocation (no user-space malloc!)
void* kmalloc(size_t size);
void kfree(void* ptr);

// System call declaration matching PDCLib expectations
ssize_t write(int fd, const void *buf, size_t count);

// Device driver write function type
typedef ssize_t (*device_write_fn)(int fd, const void *buf, size_t count);

// Initialize standard I/O devices
void kernel_io_init(void);



#include_next <stdlib.h>