//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


// Initialize HPET
void initalize_hpet(void) {
    hpet_base = 0;

    if (hpet_table_mem == 0) {
        log("\n\nHPET: Not present\n");
        return; // HPET is not present
    }

    // Read HPET base address
    dword_t *hpet_table = (dword_t *)(hpet_table_mem + 44);
    if (hpet_table[1] != 0) {
        log("\n\nHPET: 64-bit base address (unsupported)\n");
        return; // 64-bit base - we cannot access HPET registers
    }
    hpet_base = hpet_table[0];
    if (hpet_base == 0) {
        log("\n\nHPET: Invalid base address\n");
        return; // Invalid base address
    }

    // Read base values
    hpet_num_of_timers = ((mmio_ind(hpet_base + 0x00) >> 8) & 0xF);
    hpet_one_tick_how_many_nanoseconds = (mmio_ind(hpet_base + 0x04) / 1000000);
    if (hpet_one_tick_how_many_nanoseconds > 100 || hpet_one_tick_how_many_nanoseconds == 0) {
        log("\n\nHPET: Invalid nanoseconds value ");
        log_var(hpet_one_tick_how_many_nanoseconds);
        hpet_base = 0;
        return; // Invalid value
    }
    hpet_how_many_ticks_one_microsecond = (1000 / hpet_one_tick_how_many_nanoseconds);

    // Reset HPET counter and timers
    mmio_outd(hpet_base + 0x10, 0);
    mmio_outd(hpet_base + 0xF0, 0);
    mmio_outd(hpet_base + 0xF4, 0);
    for (int i = 0; i < hpet_num_of_timers; i++) {
        mmio_outd(hpet_base + 0x100 + i * 0x20 + 0, 0);
    }

    // Start counting
    mmio_outd(hpet_base + 0x10, 1);

    // Log info
    log("\n\nHPET: ");
    log_var_with_space(hpet_one_tick_how_many_nanoseconds);
    log(" nanoseconds per tick\n");
}

// Reset HPET counter
void hpet_reset_counter(void) {
    if (hpet_base == 0) {
        return;
    }

    mmio_outd(hpet_base + 0x10, 0);
    mmio_outd(hpet_base + 0xF0, 0);
    mmio_outd(hpet_base + 0xF4, 0);
    mmio_outd(hpet_base + 0x10, 0x1);
}

// Return time in microseconds
dword_t hpet_return_time_in_microseconds(void) {
    if (hpet_base == 0) {
        return 0;
    }

    return (mmio_ind(hpet_base + 0xF0) / hpet_how_many_ticks_one_microsecond);
}

