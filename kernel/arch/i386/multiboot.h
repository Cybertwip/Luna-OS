
#ifdef __cplusplus
extern "C" {
#endif

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

/* The Multiboot header. */
typedef struct multiboot_header {
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t load_end_addr;
    uint32_t bss_end_addr;
    uint32_t entry_addr;
} multiboot_header_t;

/* The section header table for ELF. */
typedef struct elf_section_header_table {
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
} elf_section_header_table_t;

/* The Multiboot information structure. */
typedef struct multiboot_info {
    uint32_t flags;         // Multiboot info version flag
    uint32_t mem_lower;     // Available memory from BIOS (below 1MB)
    uint32_t mem_upper;     // Available memory from BIOS (above 1MB)
    uint32_t boot_device;   // "root" partition
    uint32_t cmdline;       // Kernel command line
    uint32_t mods_count;    // Number of modules loaded
    uint32_t mods_addr;     // Module list address
    
    // ELF symbol table
    elf_section_header_table_t elf_sec;
    
    // Memory mapping buffer
    uint32_t mmap_length;   // Size of memory map
    uint32_t mmap_addr;     // Address of first memory map entry
    
    // Drive Info
    uint32_t drives_length; // Size of drives buffer
    uint32_t drives_addr;   // Address of first drive info
    
    // ROM configuration table
    uint32_t config_table;
    
    // Boot Loader Name
    uint32_t boot_loader_name;
    
    // APM table
    uint32_t apm_table;
    
    // Video
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
} __attribute__((packed)) multiboot_info_t;

/* Memory map entry structure */
struct multiboot_mmap_entry {
    uint32_t size;          // Size of this entry (minimum 20)
    uint32_t addr_low;      // Start address (low 32 bits)
    uint32_t addr_high;     // Start address (high 32 bits)
    uint32_t len_low;       // Length in bytes (low 32 bits)
    uint32_t len_high;      // Length in bytes (high 32 bits)
    uint32_t type;          // Memory type (1 = available RAM)
} __attribute__((packed));

#define MULTIBOOT_MEMORY_AVAILABLE     1
#define MULTIBOOT_MEMORY_RESERVED      2
#define MULTIBOOT_MEMORY_ACPI_RECLAIM  3
#define MULTIBOOT_MEMORY_NVS           4
#define MULTIBOOT_MEMORY_BADRAM        5

#endif // MULTIBOOT_H

#ifdef __cplusplus
}
#endif
