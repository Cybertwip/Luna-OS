extern "C" {
#include "sys/system.h"
#include "arch/i386/descriptor_tables.h"
#include "arch/i386/multiboot.h"
#include "mm/paging.h"
#include "mm/heap.h"
#include "mm/kmalloc.h"

#include "sys/thread.h"
#include "sys/scheduler.h"
#include "drivers/timer.h"
}

#include "luna/Engine.h"


int fn(void *arg) {
    for (;;) {
        int i;
        for (i = 0; i < 80; i++) {
            printk("a = %d\n", i);
        }
    }
    return 6;
}

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

    // uint32_t *stack = kmalloc(0x400) + 0x3F0;
    // thread_t *t = create_thread(&fn, (void *)0x567, stack);

    return 0xDEADBABA;
}
