// kernel/cxx_support.cpp

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