#pragma once

#include <stdint.h>

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA  0x01CF

inline void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %w1" : : "a"(value), "Nd"(port));
}

inline void vbe_write(uint16_t index, uint16_t value) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}
