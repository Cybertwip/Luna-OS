#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Luna {
    void* memset(void* dest, int value, size_t n);
    void* memcpy(void* dest, const void* src, size_t n);
}
