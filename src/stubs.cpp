// src/stubs.cpp
extern "C" {
    void abort() { while(1); }
    void exit(int) { while(1); }
    void _Exit(int) { while(1); }
    void* memcpy(void* dest, const void* src, unsigned n) {
        char* d = (char*)dest;
        const char* s = (const char*)src;
        while(n--) *d++ = *s++;
        return dest;
    }
}