#include "new.h"

// Global operator new
void* operator new(size_t size) noexcept {
    if (void* ptr = kmalloc(size)) {
        return ptr;
    }
    return nullptr;
}

// Global operator new[]
void* operator new[](size_t size) noexcept {
    if (void* ptr = kmalloc(size)) {
        return ptr;
    }
    return nullptr;
}

// Global operator delete
void operator delete(void* ptr) noexcept {
    kfree(ptr);
}

// Global operator delete[]
void operator delete[](void* ptr) noexcept {
    operator delete(ptr); // Reuse the same implementation
}

// Sized operator delete (optional, for completeness)
void operator delete(void* ptr, size_t) noexcept {
    ::operator delete(ptr);
}

// Sized operator delete[] (optional, for completeness)
void operator delete[](void* ptr, size_t) noexcept {
    ::operator delete[](ptr);
}