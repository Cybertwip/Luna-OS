// In memcpy.cpp (new source file)
#include "string.h"

extern "C" void* memcpy(void* dest, const void* src, size_t n) {
    char* d = static_cast<char*>(dest);
    const char* s = static_cast<const char*>(src);
    for (; n; n--) *d++ = *s++;
    return dest;
}