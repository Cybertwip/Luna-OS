#pragma once

#include "memory.h" // For kmalloc and kfree
#include <stdint.h> // For size_t

// Disable exceptions for kernel mode
#define NO_EXCEPTIONS

// Custom bad_alloc for kernel use
struct bad_alloc {
    const char* what() const noexcept { return "Allocation failed"; }
};


// Global new and delete operators
void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, size_t) noexcept;
void operator delete[](void* ptr, size_t) noexcept;

// Placement new and delete operators with correct signature
void* operator new(size_t, void* ptr) noexcept;
void* operator new[](size_t, void* ptr) noexcept;
void operator delete(void*, void*) noexcept;
void operator delete[](void*, void*) noexcept;