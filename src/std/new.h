// new.h (updated)
#pragma once

#include "memory.h"
#include <std/stdint.h>

// Disable exceptions for kernel mode
#define NO_EXCEPTIONS

// Custom bad_alloc for kernel use
struct bad_alloc {
    const char* what() const noexcept { return "Allocation failed"; }
};

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, size_t) noexcept;
void operator delete[](void* ptr, size_t) noexcept;