// mm/kmemdetect.h
#ifndef KERNEL_KMEMDETECT_H
#define KERNEL_KMEMDETECT_H

#include <stdint.h>
#include "arch/i386/multiboot.h"

uint32_t detect_physical_memory(multiboot_info_t* mb_info);

#endif