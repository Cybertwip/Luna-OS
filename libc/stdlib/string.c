#include "string.h"
#include "stddef.h"
#include "ctype.h"

void* memcpy(void* dest, const void* src, size_t n) {
    // Cast pointers to uint64_t* for 64-bit copying
    uint64_t* d64 = (uint64_t*)(dest);
    const uint64_t* s64 = (uint64_t*)(src);

    // Copy in 64-bit chunks
    size_t num_64bit_chunks = n / sizeof(uint64_t);
    for (size_t i = 0; i < num_64bit_chunks; ++i) {
        d64[i] = s64[i];
    }

    // Copy remaining bytes (if any)
    size_t remaining_bytes = n % sizeof(uint64_t);
    if (remaining_bytes) {
        char* d8 = (char*)(d64 + num_64bit_chunks);
        const char* s8 = (const char*)(s64 + num_64bit_chunks);
        for (size_t i = 0; i < remaining_bytes; ++i) {
            d8[i] = s8[i];
        }
    }

    return dest;
}

