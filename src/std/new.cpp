#include "new.h"


// Global new and delete operators
void* operator new(size_t size) {
    if (void* ptr = kmalloc(size)) {
        return ptr;
    }
#ifdef NO_EXCEPTIONS
    // Kernel-appropriate error handling (e.g., panic)
    asm volatile ("cli; hlt"); // Halt system
    return nullptr; // Unreachable, but required for compilation
#else
    throw std::bad_alloc();
#endif
}

void* operator new[](size_t size) {
    return operator new(size);
}

void operator delete(void* ptr) noexcept {
    if (ptr) kfree(ptr);
}

void operator delete[](void* ptr) noexcept {
    if (ptr) kfree(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    if (ptr) kfree(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    if (ptr) kfree(ptr);
}

// Placement new and delete operators
void* operator new(size_t size, void* ptr) noexcept {
    return ptr;
}

void* operator new[](size_t size, void* ptr) noexcept {
    return ptr;
}

void operator delete(void* ptr, void*) noexcept {
    // Placement delete does nothing
}

void operator delete[](void* ptr, void*) noexcept {
    // Placement delete[] does nothing
}

