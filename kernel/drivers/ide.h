#pragma once
#include <stdint.h>

#include <drivers/disk_interface.h>

extern void init_ide(void);

extern u8 disk_get_status(disk_e disk);

/// Initializes at disk intrface
extern u8 disk_initialize(disk_e disk);

/// Read a number of sectors from the MSD
extern u8 disk_read(disk_e disk, u8* buffer, u32 lba, u32 count);

/// Write a number of sectors to the MSD
extern u8 disk_write(disk_e disk, const u8* buffer, u32 lba, u32 count);
