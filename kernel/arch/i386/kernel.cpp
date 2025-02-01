#include "kernel.hpp"

Kernel::Kernel(uint32_t magic, multiboot_info_t* mb_info) 
: mMbInfo(mb_info) {
    if (magic != 0x2BADB002) {
        panic("0x%X\n Invalid Boot Record", magic);
    }

    init_descriptor_tables();

    kernel_io_init();

    ide = eastl::make_unique<IDE>();
    fat32 = eastl::make_unique<Fat32>(*ide);

    fat32->mount();

    init_filesystem();

    init_paging(mMbInfo);
    init_timer(20);

    // __asm__ volatile ("sti");
    // init_scheduler(init_threading());
}