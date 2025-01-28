// mm/kmemdetect.c
#include "mm/kmemdetect.h"
#include <stdbool.h>

uint32_t detect_physical_memory(multiboot_info_t* mb_info) {
    uint64_t total_size = 0; // Changed to uint64_t for larger memory support
    uint32_t mmap_length = mb_info->mmap_length;
    uint32_t mmap_addr = mb_info->mmap_addr;
    struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*)mmap_addr;

    // Check if memory map is available
    if (mb_info->flags & (1 << 6)) {
        
        while (total_size < mmap_length) {
            // Process memory map entry
            if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                // Handle available memory
                uint64_t base = ((uint64_t)entry->addr_high << 32) | entry->addr_low;
                uint64_t length = ((uint64_t)entry->len_high << 32) | entry->len_low;
                total_size += length;
            }
            
            // Move to next entry
            total_size += entry->size + sizeof(entry->size);
            entry = (struct multiboot_mmap_entry*)((uint32_t)entry + entry->size + sizeof(entry->size));
        }
    }

    // Return only the sum of available memory sizes, not including the loop's total_size
    uint64_t available_memory = 0;
    entry = (struct multiboot_mmap_entry*)mmap_addr;
    while ((uint32_t)entry < mmap_addr + mmap_length) {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t length = ((uint64_t)entry->len_high << 32) | entry->len_low;
            available_memory += length;
        }
        entry = (struct multiboot_mmap_entry*)((uint32_t)entry + entry->size + sizeof(entry->size));
    }

    // Ensure we return uint32_t as per function signature, this might truncate on very large systems
    return (uint32_t)available_memory;
}