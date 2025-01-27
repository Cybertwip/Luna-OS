#include <stddef.h>
#include <stdint.h>

namespace Luna {
void* memset(void* dest, int value, size_t n) {
    // Cast the destination pointer to a byte pointer
    uint8_t* d8 = static_cast<uint8_t*>(dest);

    // Replicate the value across all bytes of a 64-bit integer
    uint64_t value64 = static_cast<uint8_t>(value); // Ensure value is in the range 0-255
    value64 |= (value64 << 8);
    value64 |= (value64 << 16);
    value64 |= (value64 << 32);

    // Cast the destination pointer to a 64-bit pointer
    uint64_t* d64 = reinterpret_cast<uint64_t*>(d8);

    // Set memory in 64-bit chunks
    size_t num_64bit_chunks = n / sizeof(uint64_t);
    for (size_t i = 0; i < num_64bit_chunks; ++i) {
        d64[i] = value64;
    }

    // Set remaining bytes (if any)
    size_t remaining_bytes = n % sizeof(uint64_t);
    if (remaining_bytes) {
        uint8_t* d8_remaining = reinterpret_cast<uint8_t*>(d64 + num_64bit_chunks);
        for (size_t i = 0; i < remaining_bytes; ++i) {
            d8_remaining[i] = static_cast<uint8_t>(value);
        }
    }

    return dest;
}

void* memcpy(void* dest, const void* src, size_t n) {
    // Cast pointers to uint64_t* for 64-bit copying
    uint64_t* d64 = static_cast<uint64_t*>(dest);
    const uint64_t* s64 = static_cast<const uint64_t*>(src);

    // Copy in 64-bit chunks
    size_t num_64bit_chunks = n / sizeof(uint64_t);
    for (size_t i = 0; i < num_64bit_chunks; ++i) {
        d64[i] = s64[i];
    }

    // Copy remaining bytes (if any)
    size_t remaining_bytes = n % sizeof(uint64_t);
    if (remaining_bytes) {
        char* d8 = reinterpret_cast<char*>(d64 + num_64bit_chunks);
        const char* s8 = reinterpret_cast<const char*>(s64 + num_64bit_chunks);
        for (size_t i = 0; i < remaining_bytes; ++i) {
            d8[i] = s8[i];
        }
    }

    return dest;
}
}
