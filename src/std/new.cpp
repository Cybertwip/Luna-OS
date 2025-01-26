// new.cpp (updated)
#include "new.h"

void* operator new(size_t size) {
    if (void* ptr = kmalloc(size)) return ptr;
#ifdef NO_EXCEPTIONS
    // Kernel-appropriate error handling (e.g., panic)
    asm volatile ("cli; hlt"); // Halt system
    return nullptr;
#else
    throw bad_alloc();
#endif
}

void* operator new[](size_t size) {
    return operator new(size);
}

void operator delete(void* ptr) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr) noexcept {
    kfree(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    kfree(ptr);
}