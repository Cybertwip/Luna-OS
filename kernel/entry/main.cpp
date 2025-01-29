extern "C" {
#include "arch/i386/descriptor_tables.h"
#include "arch/i386/multiboot.h"
#include "sys/scheduler.h"
#include "drivers/timer.h"
}

#include "luna/Engine.h"

#include <stdio.h>

int main(uint32_t magic, multiboot_info_t* mb_info) {
    if (magic != 0x2BADB002) {
        panic("Invalid Boot Record");
        // Optionally: print error, halt, or return an error code
        return -1; // Return error code
    }

    init_descriptor_tables();

    init_paging(mb_info);

    init_timer(20);

    __asm__ volatile ("sti");
    init_scheduler(init_threading());

    GameEngine engine;
    engine.run();

    return 0xDEADBABA;
}
