// kernel_stdlib.h
#pragma once

// Kernel-specific memory allocation (no user-space malloc!)
void* kmalloc(size_t size);
void kfree(void* ptr);
