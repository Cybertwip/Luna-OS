/*
 * Copyright (c) 2013, Alyx Wolcott <contact@alyxw.me>
 * Copyright (c) 2013, Justin Crawford <Justasic@gmail.com>
 * Copyright (c) 2013, Piero Barbagelata <lordofsraam@hotmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice 
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "string.h"
#include "stddef.h"
#include "ctype.h"
#include "private/platform.h"


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

