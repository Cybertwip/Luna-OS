#include "vbe.h"

void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %w1" : : "a"(value), "Nd"(port));
}

void vbe_write(uint16_t index, uint16_t value) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}

