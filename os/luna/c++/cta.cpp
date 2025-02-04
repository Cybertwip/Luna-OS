extern "C" void* __dso_handle = nullptr;

// Implement __cxa_atexit
extern "C" int __cxa_atexit(void (*destructor)(void*), void* arg, void* dso_handle) {
    // Implement your own atexit functionality if needed
    return 0; // Return 0 on success
}