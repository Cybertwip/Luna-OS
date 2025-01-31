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
#define MAX_FDS 1024  // Adjust this value as needed

ssize_t write(int fd, const void *buf, size_t count);

int close(int fd);
int unlink(const char *pathname);
void _exit(int status);

// Device driver write function type
typedef ssize_t (*device_write_fn)(int fd, const void *buf, size_t count);

// Initialize standard I/O devices
void kernel_io_init(void);

extern void *luna_malloc(size_t size);

extern void *luna_realloc(void *ptr, size_t size);

extern void luna_free(void *ptr);

