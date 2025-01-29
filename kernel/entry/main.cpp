extern "C" {
#include "arch/i386/descriptor_tables.h"
#include "arch/i386/multiboot.h"
#include "sys/scheduler.h"
#include "drivers/timer.h"
#include "drivers/ide.h"
#include "drivers/voltron.h"

#include <stdlib.h>
}

#include "luna/Engine.h"



int main(uint32_t magic, multiboot_info_t* mb_info) {
    if (magic != 0x2BADB002) {
        panic("Invalid Boot Record");
        // Optionally: print error, halt, or return an error code
        return -1; // Return error code
    }

    init_descriptor_tables();

    kernel_io_init();

    init_ide();

    init_paging(mb_info);

    init_timer(20);

    __asm__ volatile ("sti");
    init_scheduler(init_threading());

    disable_paging();
    uint8_t sector_buffer[512];

    // ahci_init();
    // ahci_read_sector(0, 0, sector_buffer);


    // Example: Print 512 bytes of the sector
    printk("%s", "\n");
    for (int i = 0; i < 512; i++) {
        printk("%02x ", sector_buffer[i]);
    }

    
    switch_page_directory(kernel_directory);

    // GameEngine engine;
    // engine.run();

    return 0xDEADBABA;
}
