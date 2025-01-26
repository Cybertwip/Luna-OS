#pragma once

#include "memory.h"
#include <stdint.h> // for size_t

// Custom exception for allocation failures
struct bad_alloc {
    const char* what() const noexcept {
        return "bad allocation";
    }
};

// Declare the operators
void* operator new(size_t size);
void operator delete(void* ptr) noexcept;
void operator delete(void* ptr, size_t) noexcept;
void operator delete[](void* ptr) noexcept;
