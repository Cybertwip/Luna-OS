#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C" void* memset(void* dest, int value, size_t n);
extern "C" void* memcpy(void* dest, const void* src, size_t n);
