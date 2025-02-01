#pragma once

#include <arch/i386/multiboot.h>
#include <arch/i386/descriptor_tables.h>

#include <kernel.h>

#include <sys/scheduler.h>
#include <drivers/timer.h>
#include <drivers/voltron.h>
#include <drivers/ide.h>
#include <drivers/fat32.h>
#include <mm/paging.h>

#include <EASTL/unique_ptr.h>

class Kernel {
public:
    Kernel(uint32_t magic, multiboot_info_t* mb_info);

    void init();

private:
    eastl::unique_ptr<IDE> ide;
    eastl::unique_ptr<Fat32> fat32;
    multiboot_info_t* mMbInfo;
};