#include "voltron.h"
#include <drivers/ide.h>  
#include <drivers/fat32.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct PartitionEntry {
    uint8_t status;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_end[3];
    uint32_t lba_start;  // Starting sector of the FAT32 partition
    uint32_t num_sectors;
} __attribute__((packed));

bool init_filesystem() {

    uint8_t mbr[512];
    ide_read_blocks(0, 0, 1, mbr); // Read MBR

    // Check MBR signature
    if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
        printk("Invalid MBR signature.\n");
        return false;
    }

    // Parse partition entries (4 entries total)
    struct PartitionEntry *partitions = (struct PartitionEntry*)(mbr + 0x1BE);
    for (int i = 0; i < 4; i++) {
        if (partitions[i].type == 0x0B || partitions[i].type == 0x0C) { // FAT32 type
            uint32_t fat32_start_lba = partitions[i].lba_start;
            printk("FAT32 partition starts at LBA %u\n", fat32_start_lba);
            
            return FAT32_Init(fat32_start_lba);
        }
    }

    printk("No FAT32 partition found.\n");
    return false;
}

void list_root() {
    FAT32_File root;
    if (FAT32_Open(&root, "/")) {
        FAT32_File entry;
        while (FAT32_ReadDir(&root, &entry)) {
            printk("%s %s %d bytes\n", 
                   (entry.attributes & 0x28) ? "[DIR]" : "[FILE]",
                   entry.name,
                   entry.size);
        }
    }
}