#include <new>

#include <stdlib.h>

// Global new and delete operators
void* operator new(size_t size) {
    if (void* ptr = malloc(size)) {
        return ptr;
    }
#ifdef NO_EXCEPTIONS
    // Kernel-appropriate error handling (e.g., panic)
    __asm__ volatile ("cli; hlt"); // Halt system
    return nullptr; // Unreachable, but required for compilation
#else
    throw std::bad_alloc();
#endif
}

void* operator new[](size_t size) {
    return operator new(size);
}

void operator delete(void* ptr) noexcept {
    if (ptr) free(ptr);
}

void operator delete[](void* ptr) noexcept {
    if (ptr) free(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    if (ptr) free(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    if (ptr) free(ptr);
}

// Placement new and delete operators
void* operator new(size_t, void* ptr) noexcept {
    return ptr;
}

void* operator new[](size_t, void* ptr) noexcept {
    return ptr;
}

void operator delete(void*, void*) noexcept {
    // Placement delete does nothing
}

void operator delete[](void*, void*) noexcept {
    // Placement delete[] does nothing
}
