// DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//                    Version 2, December 2004
//  
// Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
// 
// Everyone is permitted to copy and distribute verbatim or modified
// copies of this license document, and changing it is allowed as long
// as the name is changed.
//  
//            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
// 
//  0. You just DO WHAT THE FUCK YOU WANT TO.

#ifndef DISK_INTERFACE_H
#define DISK_INTERFACE_H

#include <drivers/fat_types.h>
#include <stdbool.h>

/// Add new physical disk here
typedef enum {
	DISK_HD_ATA
} disk_e;


typedef struct {
    unsigned int minor;
    bool initialized;
} hd_ata;

/// Returns the status of the MSD (mass storage device)
u8 disk_get_status(disk_e disk);

/// Initializes at disk intrface
u8 disk_initialize(disk_e disk);

/// Read a number of sectors from the MSD
u8 disk_read(disk_e disk, u8* buffer, u32 lba, u32 count);

/// Write a number of sectors to the MSD
u8 disk_write(disk_e disk, const u8* buffer, u32 lba, u32 count);

extern u8 hd_ata_get_status(hd_ata* hd);

extern u8 hd_ata_protocol_config(hd_ata* hd);

extern u8 hd_ata_protocol_read(hd_ata* hd, u8* buffer, u32 lba, u32 count);

extern u8 hd_ata_protocol_write(hd_ata* hd, const u8* buffer, u32 lba, u32 count);

#endif