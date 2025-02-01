// DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//                    Version 2, December 2004
//  
// Copyright (C) 2025 Victor Lopez
// 
// Everyone is permitted to copy and distribute verbatim or modified
// copies of this license document, and changing it is allowed as long
// as the name is changed.
//  
//            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
// 
//  0. You just DO WHAT THE FUCK YOU WANT TO.

#include <drivers/disk_interface.h>
#include <drivers/ide.h>

/// Make a clobal HD ATA variable
hd_ata hd_slot_0 = { .minor = 0, .initialized = false };

u8 disk_get_status(disk_e disk) {
	switch (disk) {
		case DISK_HD_ATA: {
			return (u8)hd_ata_get_status(&hd_slot_0);
		}
	}
	return 0;
}

u8 disk_initialize(disk_e disk) {
	switch (disk) {
		case DISK_HD_ATA: {
			return (u8)hd_ata_protocol_config(&hd_slot_0);
		}
	}
	return 0;
}

u8 disk_read(disk_e disk, u8* buffer, u32 lba, u32 count) {
	switch (disk) {
		case DISK_HD_ATA: {
			return hd_ata_protocol_read(&hd_slot_0, buffer, lba, count);
		}
	}
	return 0;
}

u8 disk_write(disk_e disk, const u8* buffer, u32 lba, u32 count) {
	switch (disk) {
		case DISK_HD_ATA: {
			return hd_ata_protocol_write(&hd_slot_0, buffer, lba, count);
		}
	}
	return 0;
}
