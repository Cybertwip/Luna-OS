// kernel/cxx_support.cpp
#include <stdint.h<

extern "C" uint64_t __udivdi3(uint64_t dividend, uint64_t divisor) {
    uint64_t quotient = 0;
    uint64_t remainder = 0;

    for (int i = 63; i >= 0; --i) {
        remainder <<= 1;
        remainder |= (dividend >> i) & 1;

        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= (1ULL << i);
        }
    }

    return quotient;
}


extern "C" {
    // Dummy __dso_handle for global destructors
    void* __dso_handle = nullptr;

    // Dummy __cxa_atexit for registering global destructors
    int __cxa_atexit(void (*destructor)(void*), void* arg, void* dso_handle) {
        // Do nothing: we don't support global destructors in the kernel
        (void)destructor;
        (void)arg;
        (void)dso_handle;
        return 0;
    }

    // Dummy __cxa_finalize for finalizing global destructors
    void __cxa_finalize(void* dso_handle) {
        // Do nothing: we don't support global destructors in the kernel
        (void)dso_handle;
    }

    // Dummy __cxa_pure_virtual for pure virtual functions
    void __cxa_pure_virtual() {
        // Panic or log an error if a pure virtual function is called
        while (1) {} // Hang the kernel
    }
    
}