#include "fat32.h"

#include <stddef.h>
#include <time.h>
#include <math.H>

#include "ansi_colors.h"

#include <drivers/ide.h>
#include <drivers/ufat.h>
#include <mm/kmalloc.h>
#include <sys/system.h>

#include <luna/std/string.h>
#include <ctype.h>


/// Most of the FAT32 file system functions returns one of these status codes
typedef enum {
	FSTATUS_OK,
	FSTATUS_ERROR,
	FSTATUS_ALREADY_EXISTS,
	FSTATUS_NO_VOLUME,
	FSTATUS_PATH_ERR,
	FSTATUS_EOF
} fstatus;

struct volume_s {
	struct volume_s* next;
	
	// The first label is 11-bytes and located in the BPB, while the sectondary
	// label is introduced in the root directory. The BPB label contains 13 
	// characters while the root label can contain 13 characters.
	char label[13];
	char letter;
	
	// FAT32 info
	u16 sector_size;
	u8 cluster_size;
	u32 total_size;
	u32 fat_lba;
	u32 fsinfo_lba;
	u32 data_lba;
	u32 root_lba;
    u32 lba;
	
	// All file system operations require a 512-byte buffer for storing the
	// current sector.
	u8 buffer[512];
	u32 buffer_lba;
	disk_e disk;
	u32 buffer_dirty;
	
	char lfn[256];
	u8 lfn_size;
	
};

struct dir_s {
	u32 sector;
	u32 cluster;
	u32 rw_offset;
	
	u32 start_sect;
	u32 size;
	struct volume_s* vol;
};

struct file_s {
	u32 sector;
	u32 cluster;
	u32 rw_offset;
	u32 size;
	u32 start_sect;
	u32 glob_offset;
	struct volume_s* vol;
};

/// This structure will contain all information needed for a file or a folder. 
/// It is mainly used to read directory entries from a path
struct info_s {
	
	// By default this code supports long file name entries (LFN) up to 
	// 256 characters. The same buffer will be used for LFN and SFN entries.
	char name[256];
	u8 name_length;
	
	// The attribute field apply to a file or a folder
	// 
	// Bit 0 - Read-only
	// Bit 1 - Hidden
	// Bit 2 - System (do not mess with these directories)
	// Bit 3 - Volume label
	// Bit 4 - Subdirectory
	// Bit 5 - Archeive
	// Bit 6 - Device
	u8	attribute;
	
	// Time and date properties
	u8	c_time_tenth;
	u16 c_time;
	u16 c_date;
	u16 a_date;
	u16 w_time;
	u16 w_date;
	
	// Contains the total size of a file or a folder
	// A folder has 
	u32 size;
};

/// The classical generic MBR located at sector zero at a MSD contains four 
/// partition fields. This structure describe one partition. 
struct partition_s {
	u32 lba;
	u32 size;
	u8 status;
	u8 type;
};

/// Format structure
struct fat_fmt_s {
	u32 allocation_size;
	u32 allignment;
	u32 quick_format;
};

//------------------------------------------------------------------------------
// Microsoft FAT32 spesification
// Due to Microsoft releasing the licensing on the FAT LFN usage, this code will 
// use LFN instead of SFN. It will not have SFN support since it is not meant
// for smaller systems.
//------------------------------------------------------------------------------

/// MBR and boot sector
#define MBR_BOOTSTRAP		0
#define MBR_BOOTSTRAP_SIZE	446
#define MBR_PARTITION		446
#define MBR_PARTITION_SIZE	16
#define MBR_BOOT_SIG		510
#define MBR_BOOT_SIG_VALUE	0xAA55

#define PAR_STATUS			0
#define PAR_TYPE			4
#define PAR_LBA				8
#define PAR_SIZE			12

/// Old BPB and BS
#define BPB_JUMP_BOOT		0
#define BPB_OEM				3
#define BPB_SECTOR_SIZE		11
#define BPB_CLUSTER_SIZE	13
#define BPB_RSVD_CNT		14
#define BPB_NUM_FATS		16
#define BPB_ROOT_ENT_CNT	17
#define BPB_TOT_SECT_16		19
#define BPB_MEDIA			21
#define BPB_FAT_SIZE_16		22
#define BPB_SEC_PER_TRACK	24
#define BPB_NUM_HEADS		26
#define BPB_HIDD_SECT		28
#define BPB_TOT_SECT_32		32

/// New BPB and BS applying for FAT12 and FAT16
#define BPB_16_DRV_NUM		36
#define BPB_16_RSVD1		37
#define BPB_16_BOOT_SIG		38
#define BPB_16_VOL_ID		39
#define BPB_16_VOL_LABEL	43
#define BPB_16_FSTYPE		54

// New BPB and BS applying for FAT32
#define BPB_32_FAT_SIZE		36
#define BPB_32_EXT_FLAGS	40
#define BPB_32_FSV			42
#define BPB_32_ROOT_CLUST	44
#define BPB_32_FSINFO		48
#define BPB_32_BOOT_SECT	50
#define BPB_32_RSVD			52
#define BPB_32_DRV_NUM		64
#define BPB_32_RSVD1		65
#define BPB_32_BOOT_SIG		66
#define BPB_32_VOL_ID		67
#define BPB_32_VOL_LABEL	71
#define BPB_32_FSTYPE		82

/// Directory entry defines
#define SFN_NAME			0
#define SFN_ATTR			11
#define SFN_NTR				12
#define SFN_CTIME_TH		13
#define SFN_CTIME			14
#define SFN_CDATE			16
#define SFN_ADATE			18
#define SFN_CLUSTH			20
#define SFN_WTIME			22
#define SFN_WDATE			24
#define SFN_CLUSTL			26
#define SFN_FILE_SIZE		28

#define LFN_SEQ				0
#define LFN_SEQ_MSK			0x1F
#define LFN_NAME_1			1
#define LFN_ATTR			11
#define LFN_TYPE			12
#define LFN_CRC				13
#define LFN_NAME_2			14
#define LFN_NAME_3			28

#define ATTR_RO				0x01
#define ATTR_HIDD			0x02
#define ATTR_SYS			0x04
#define ATTR_VOL_LABEL		0x08
#define ATTR_DIR			0x10
#define ATTR_ARCH			0x20
#define ATTR_LFN			0x0F

/// FSinfo structure
#define INFO_CLUST_CNT		488
#define INFO_NEXT_FREE		492

#define FAT32_EOC 0x0FFFFFFF

#define print printk

struct ufat uf;
struct ufat_device device;

struct fat_device_s {
	struct IDE* mIde;
	struct volume_s* mVol;
};

int fat_device_read(const struct ufat_device *dev, ufat_block_t start,
                    ufat_block_t count, void *buffer) {

	struct fat_device_s* fat_device = (struct fat_device_s*)dev->data;
    // Note: 'buffer' is const void* for reading
    return fat_device->mIde->read(buffer, fat_device->mVol->lba + start, count);
}

int fat_device_write(const struct ufat_device *dev, ufat_block_t start,
                     ufat_block_t count, const void *buffer) {
	struct fat_device_s* fat_device = (struct fat_device_s*)dev->data;
    // Note: 'buffer' is const void* for reading
    return fat_device->mIde->write(buffer, fat_device->mVol->lba + start, count);
}

/// Buffer and bitmask used for volume mounting. When a partition on the MSD 
/// contains a valid FAT32 file system, a FAT32 volume is dynamically allocated
/// and added to the linked list with base `volume_base`. The bitmask ensures 
/// a unique volume letter for each volume 
static struct volume_s* volume_base;
static u32 volume_bitmask;

/// Temporary buffer used in the mounting process, specifically for retrieving
/// the MBR boot sector and BPB sector for FAT32 recognition
static u8 mount_buffer[512];

/// UCS-2 offsets used in long file name (LFN) entries
static const u8 lfn_lut[] = {1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30};
	
/// Remove
static const char file_size_ext[] = {'k', 'M', 'G'};

/// Microsoft uses lookup tables for an uninitialized volume. This is because
/// the FAT12/16/32 type is dependent on the number of cluster, and the sectors
/// are dependent on the volume size.
///
/// This look-up table does ONLY apply when:
///		- sector size equals 512
///		- the reserved sector count equals 32
///		- number of FAT's equals 2
struct clust_size_s { u32 sector_cnt; u32 clust_size; };
static const struct clust_size_s cluster_size_lut[] = {
	{      66600,	0  },			// Disks up to 32.5 MB	
	{     532480,	1  },			// Disks up to 260 MB	, 0.5k clusters
	{   16777216,	8  },			// Disks up to 8 GB		, 4k clusters
	{   33554432,	16 },			// Disks up to 16 GB	, 8k clusters
	{   67108864,	32 },			// Disks up to 32 GB	, 16k clusters
	{ 0xFFFFFFFF,	64 }			// Disks > 32 GB		, 32k clusters<
};


/// Private prototypes
static void fat_store32(void* dest, u32 value);
static void fat_store16(void* dest, u16 value);
static u32 fat_load32(const void* src);
static u16 fat_load16(const void* src);
static u8 fat_dir_lfn_cmp(const u8* lfn, const char* name, u32 size);
static u8 fat_table_get(IDE& ide, struct volume_s* vol, u32 cluster, u32* fat);
static u8 fat_read(IDE& ide, struct volume_s* vol, u32 lba);
static u8 fat_flush(IDE& ide, struct volume_s* vol);
static inline u32 fat_clust_to_sect(struct volume_s* vol, u32 clust);
static fstatus fat_follow_path(IDE& ide, struct dir_s* dir, const char* path, u32 length);
static fstatus fat_get_vol_label(IDE& ide, struct volume_s* vol, char* label);

/// Volume functions
struct volume_s* volume_get_first(void);
struct volume_s* volume_get(char letter);



/// Compares two memory blocks with size `count`
static u8 fat_memcmp(const void* src_1, const void* src_2, u32 count) {
	if (count == 0) {
		return 1;
	}
	const u8* src_1_ptr = (const u8 *)src_1;
	const u8* src_2_ptr = (const u8 *)src_2;
	
	do {
		if (*src_1_ptr != *src_2_ptr) {
			return 0;
		}
		src_1_ptr++;
		src_2_ptr++;
	} while (--count);
	
	return 1;
}
/// Calculates the SFN checksum based on the 8.3 short file name
static u8 fat_dir_sfn_crc(const u8* sfn) {
	u8 crc = 0;
	u8 count = 11;
	do {
		crc = ((crc & 1) << 7) + (crc >> 1) + *sfn++;
	} while (--count);
	
	return crc;
}


/// Store a 32-bit value in LE format
static void fat_store32(void* dest, u32 value) {
	u8* dest_ptr = (u8 *)dest;
	*dest_ptr++ = (u8)value;
	value >>= 8;
	*dest_ptr++ = (u8)value;
	value >>= 8;
	*dest_ptr++ = (u8)value;
	value >>= 8;
	*dest_ptr++ = (u8)value;
}

/// Store a 16-bit value in LE format
static void fat_store16(void* dest, u16 value) {
	u8* dest_ptr = (u8 *)dest;
	*dest_ptr++ = (u8)value;
	value >>= 8;
	*dest_ptr++ = (u8)value;
}

/// Load a 32-bit value from `src` in LE format
static u32 fat_load32(const void* src) {
	u32 value = 0;
	const u8* src_ptr = (const u8 *)src;
	value |= *src_ptr++;
	value |= (*src_ptr++ << 8);
	value |= (*src_ptr++ << 16);
	value |= (*src_ptr++ << 24);
	return value;
}

/// Load a 16-bit value from `src` in LE format
static u16 fat_load16(const void* src) {
	u16 value = 0;
	const u8* src_ptr = (const u8 *)src;
	value |= *src_ptr++;
	value |= (*src_ptr++ << 8);
	return value;
}


/// Add a volume to the system volumes and assign a letter to it
static u8 fat_volume_add(struct volume_s* vol) {
	if (volume_base == NULL) {
		volume_base = vol; 
	} else {
		struct volume_s* vol_it = volume_base;
		while (vol_it->next != NULL) {
			vol_it = vol_it->next;
		}
		vol_it->next = vol;
	}
	vol->next = NULL;
	
	// Assign a letter to the volume based on the bitmask
	for (u8 i = 0; i < 32; i++) {
		if ((volume_bitmask & (1 << i)) == 0) {
			volume_bitmask |= (1 << i);
			vol->letter = 'C' + i;
			break;
		}
	}
	return 1;
}

/// Remove a volume from the system volumes. This functions does NOT delete the 
/// memory
static u8 fat_volume_remove(char letter) {
	struct volume_s* curr;
	if (volume_base == NULL) {
		return 0;
	} else if (volume_base->letter == letter) {
		curr = volume_base;
		volume_base = volume_base->next;
	} else {
		struct volume_s* prev = volume_base;
		curr = volume_base->next;
		
		while (curr != NULL) {
			if (curr->letter == letter) {
				break;
			}
			prev = curr;
			curr = curr->next;
		}
		
		if (curr == NULL) return 0;
		prev->next = curr->next;
	}
	
	// Clear the bit set in the bitmask
	u8 bit_pos = curr->letter - 'C';
	volume_bitmask &= ~(1 << bit_pos);
	
	return 1;
}


/// Checks for a valid FAT32 file system on the given partition. The `bpb`
/// should point to a buffer containing the first sector in this parition. 
static u8 fat_search(const u8* bpb) {
	// Check the BPB boot signature
	if (fat_load16(bpb + 510) != 0xAA55) return 0;
	
	// A valid FAT file system will have the "FAT" string in either the FAT16
	// boot sector, or in the FAT32 boot sector. This does NOT indicate the
	// FAT file system fype
	if (!fat_memcmp(bpb + BPB_32_FSTYPE, "FAT", 3)) {
		if (!fat_memcmp(bpb + BPB_16_FSTYPE, "FAT", 3)) {
			return 0;
		}
	}
	
	// A FAT12, FAT16 or FAT32 file system is present. The type is determined 
	// by the count of data cluster.
	u32 root_sectors = ((fat_load16(bpb + BPB_ROOT_ENT_CNT) * 32) + 
		(fat_load16(bpb + BPB_SECTOR_SIZE) - 1)) / 
		(fat_load16(bpb + BPB_SECTOR_SIZE) - 1);
	
	u32 fat_size = (fat_load16(bpb + BPB_FAT_SIZE_16)) ? 
		(u32)fat_load16(bpb + BPB_FAT_SIZE_16) : 
		fat_load32(bpb + BPB_32_FAT_SIZE);
		
	u32 tot_sect = (fat_load16(bpb + BPB_TOT_SECT_16)) ? 
		(u32)fat_load16(bpb + BPB_TOT_SECT_16) :
		fat_load32(bpb + BPB_TOT_SECT_32);
		
	u32 data_sectors = tot_sect - (fat_load16(bpb + BPB_RSVD_CNT) + 
		(bpb[BPB_NUM_FATS] * fat_size) + root_sectors);
		
	u32 data_clusters = data_sectors / bpb[BPB_CLUSTER_SIZE];
	
	// Only FAT32 is supported
	if (data_clusters < 65525) {
		return 0;
	}
	return 1;
}



/// Move the `dir` pointer to the next 32-byte directory entry
static u8 fat_dir_get_next(IDE& ide, struct dir_s* dir) {
	// Update the rw offset to point to the next 32-byte entry
	dir->rw_offset += 32;
	
	// Check for sector overflow
	if (dir->rw_offset >= dir->vol->sector_size) {
		dir->rw_offset -= dir->vol->sector_size;
		dir->sector++;
		
		// Check for cluster overflow
		if (dir->sector >= (fat_clust_to_sect(dir->vol, dir->cluster) +
				dir->vol->cluster_size)) {
			
			// Get the next cluster from the FAT table	
			u32 new_cluster;
			if (!fat_table_get(ide, dir->vol, dir->cluster, &new_cluster)) {
				return 0;
			}
			
			// Check if the FAT table entry is the EOC. The FAT table entry
			// will in these cases be either EOC or date clusters. No need 
			// to check for bad clusters. 
			u32 eoc_value = new_cluster & 0xFFFFFFF;
			if ((eoc_value >= 0xFFFFFF8) && (eoc_value <= 0xFFFFFFF)) {
				return 0;
			}
			
			// Update the sector LBA from the new cluster number
			dir->cluster = new_cluster;
			dir->sector = fat_clust_to_sect(dir->vol, dir->cluster);
		}
	}
	return 1;
}


/// Returns the 32-bit FAT entry corresponding with the cluster number
static u8 fat_table_get(IDE& ide, struct volume_s* vol, u32 cluster, u32* fat_entry) {
	// Calculate the sector LBA from the FAT table base address
	u32 start_sect = vol->fat_lba + cluster / 128;
	u32 start_off = cluster % 128;
	
	if (!fat_read(ide, vol, start_sect)) {
		return 0;
	}
	*fat_entry = fat_load32(vol->buffer + start_off * 4);
	return 1;
}

/// Caches the `lba` sector in the volume buffer. If this buffer is already
/// present, the function returns `1`. Any dirty buffer will be written back
/// before the next sector is fetched. Return `0` in case of hardware fault
static u8 fat_read(IDE& ide, struct volume_s* vol, u32 lba) {
	
	// Check if the sector is already cached
	if (vol->buffer_lba != lba) {
		// Flush any dirty buffer back to the storage device
		if (!fat_flush(ide, vol)) {
			return 0;
		}
		// Cache the next sector
		if (!ide.read(vol->buffer, lba, 1)) {
			return 0;
		}
		vol->buffer_lba = lba;
	}
	return 1;
}

/// Clean a volume buffer
static u8 fat_flush(IDE& ide, struct volume_s* vol) {
	if (vol->buffer_dirty) {
		if (!ide.write(vol->buffer, vol->buffer_lba, 1)) {
			return 0;
		}
		vol->buffer_dirty = 0;
	}
	return 1;
}

/// Convert a relative cluster number to the absolute LBA address
static inline u32 fat_sect_to_clust(struct volume_s* vol, u32 sect) {
	return ((sect - vol->data_lba) / vol->cluster_size) + 2;
}

/// Convert an absolute LBA address to the relative cluster number
static inline u32 fat_clust_to_sect(struct volume_s* vol, u32 clust) {
	return ((clust - 2) * vol->cluster_size) + vol->data_lba;
}

/// Compares `size` characters from two strings without case sensitivity
static u8 fat_dir_sfn_cmp(const char* sfn, const char* name, u8 size) {
	if (size > 8) {
		size = 8;
	}
	do {
		char tmp_char = *name;
		
		// A lowercase characters is converted to an uppercase
		if (tmp_char >= 'a' && tmp_char <= 'z') {
			tmp_char -= 32;
		}
		if (tmp_char != *sfn) {
			return 0;
		}
		sfn++;
		name++;
	} while (--size);
	return 1;
}

/// Compartes a LFN entry against a given file name. `name` is the full string 
/// to be comared and `lfn` is only one LFN entry. The code will just compare
/// the affected fragment of the `name` string.
static u8 fat_dir_lfn_cmp(const u8* lfn, const char* name, u32 size) {
	
	// Compute the `name` offset of a fragment which should match the LFN name
	u8 name_off = 13 * ((lfn[LFN_SEQ] & LFN_SEQ_MSK) - 1);
	
	for (u8 i = 0; i < 13; i++) {
		// The first empty UCS-2 character will contain 0x0000 and the rest will
		// contain 0xFFFF. 
		if (lfn[lfn_lut[i]] == 0x00 || lfn[lfn_lut[i]] == 0xff) {
			break;
		}
		// Compare the first charater in the UCS-2. This will typically be a
		// ordinary ASCII character
		if (lfn[lfn_lut[i]] != name[name_off + i]) {
			return 0;
		}
	}
	return 1;
}


/// Takes in a pointer to a directory (does not need to be the leading entry)
/// and tries to find a directory entry matching `name`
static u8 fat_dir_search(IDE& ide, struct dir_s* dir, const char* name, u32 size) {
	
	// A search start from the leading entry
	if (dir->start_sect != dir->sector) {
		dir->sector = dir->start_sect;
		dir->cluster = fat_sect_to_clust(dir->vol, dir->sector);
		dir->rw_offset = 0;
	}
	
	u8 lfn_crc = 0;
	u8 lfn_match = 1;
	u8 match = 0;
	
	while (1) {
		// Update the buffer if needed
		if (!fat_read(ide, dir->vol, dir->sector)) {
			return 0;
		}
		u8* buffer = dir->vol->buffer;
		u32 rw_offset = dir->rw_offset;
		
		u8 sfn_tmp = buffer[rw_offset];
		// Check for the EOD marker
		if (sfn_tmp == 0x00) {
			break;
		}
		// Only allow used folders to be compared
		if (!((sfn_tmp == 0x00) || (sfn_tmp == 0x05) || (sfn_tmp == 0xE5))) {
			
			// Check if the entry pointed to by `dir` is a LFN or a SFN
			if ((buffer[rw_offset + SFN_ATTR] & ATTR_LFN) == ATTR_LFN) {
				
				// If the LFN name does not match the input
				if (!fat_dir_lfn_cmp(buffer + rw_offset, name, size)) {
					// TODO:
					// LFN contains the sequence number so we could jump to
					// the next entry right away. This will speed up directory
					// search around x3 times
					lfn_match = 0;
				}
				lfn_crc = buffer[rw_offset + LFN_CRC];
			} else {
				
				// The current entry is a SFN
				if (lfn_crc && lfn_match) {
					// The current SFN entry is the last in a sequence of LFN's
					if (lfn_crc == fat_dir_sfn_crc(buffer + rw_offset)) {
						match = 1;
					}
				} else {
					// Compare `name` with the SFN 8.3 file name
					if (fat_dir_sfn_cmp((char *)buffer + rw_offset, name, size)) {
						match = 1;
					}
				}
				if (match) {
					// Update the `dir` pointer
					dir->cluster = (fat_load16(buffer + rw_offset +
						SFN_CLUSTH) << 16) | fat_load16(buffer +
						rw_offset + SFN_CLUSTL);
					dir->sector = fat_clust_to_sect(dir->vol, dir->cluster);
					dir->start_sect = dir->sector;
					dir->size = fat_load32(buffer + rw_offset + SFN_FILE_SIZE);
					dir->rw_offset = 0;

					return 1;
				}
				lfn_match = 1;
				lfn_crc = 0;
			}
		}
		// Get the next 32-byte directory entry
		if (!fat_dir_get_next(ide, dir)) {
			return 0;
		}
	}
	return 0;
}



/// Follows the `path` and returns the `dir` object pointing to the last found 
/// folder. If not found the functions returns `0`, but the `dir` object may
/// still be changed
/// 
/// Path should be on the form: C:/home/usr/bin/chrome.exe
static fstatus fat_follow_path(IDE& ide, struct dir_s* dir, const char* path, u32 length) {
	
	// Volume object is determined from the first character
	struct volume_s* vol = volume_get(*path++);
	if (vol == NULL) {
		return FSTATUS_NO_VOLUME;
	}
	// Rewind the `dir` object to the root directory
	dir->vol = vol;
	dir->start_sect = dir->sector = vol->root_lba;
	dir->cluster = fat_clust_to_sect(vol, vol->root_lba);
	dir->rw_offset = 0;
	
	// Check for the colon
	if (*path++ != ':') {
		return FSTATUS_PATH_ERR;
	}
	if (*path != '/') {
		return FSTATUS_PATH_ERR;
	}
	
	// `frag_ptr` and `frag_size` will contain one fragment of the path name
	const char* frag_ptr;
	u8 frag_size;
	while (1) {
		
		// Search for the first `/`
		while (*path && (*path != '/')) {
			path++;
		}
		// TODO: This is not right is it?
		// Check if the next fragment exist
		if (*path++ == '\0') break;
		if (*path == '\0') break;
		
		// `path` points to the first character in the current name fragment
		const char* tmp_ptr = path;
		frag_ptr = path;
		frag_size = 0;
		
		while ((*tmp_ptr != '\0') && (*tmp_ptr != '/')) {
						
			// The current fragment describes is a file, However, the name 
			// fragment before it has been found
			if (*tmp_ptr == '.') {
				return FSTATUS_OK;
			}
			frag_size++;
			tmp_ptr++;
		}
		
		// Now `frag_ptr` will point to the first character in the name
		// fragment, and `frag_size` will contain the size

		print("Searching for directory: " WHT);

		print("%s: %d", frag_ptr, frag_size);
		print("\n");

		// Search for a matching directory name in the current directory. If
		// matched, the `fat_dir_search` will update the `dir` pointer as well
		if (!fat_dir_search(ide, dir, frag_ptr, frag_size)) {
			print(RED "Directory not found\n" WHT);
			return FSTATUS_PATH_ERR;
		}
	}	
	return FSTATUS_OK;
}

/// Get the volume label stored in the root directory. This is the one used by
/// Microsoft, not the BPB volume ID
static fstatus fat_get_vol_label(IDE& ide, struct volume_s* vol, char* label) {	
	// Make a directory object pointing to the root directory
	struct dir_s dir;
	dir.sector = vol->root_lba;
	dir.rw_offset = 0;
	dir.cluster = fat_sect_to_clust(vol, dir.sector);
	
	// The volume label is a SFN entry in the root directory with bit 3 set in
	// the attribute field. Volume label is limited to 13 uppercase characters
	while (1) {
		if (!fat_read(ide, vol, dir.sector)) {
			return FSTATUS_ERROR;
		}
		
		// Check if the attribute is volume label
		u8 attribute = vol->buffer[dir.rw_offset + SFN_ATTR];
		if (attribute & ATTR_VOL_LABEL) {
			
			// LFN file name entries are also marked with volume label
			if ((attribute & ATTR_LFN) != ATTR_LFN) {
				const char* src = (const char *)(vol->buffer + dir.rw_offset);
				
				for (u8 i = 0; i < 11; i++) {
					*label++ = *src++;
				}
				return 1;
			}
		}
		// Get the next directory
		if (!fat_dir_get_next(ide, &dir)) {
			return FSTATUS_ERROR;
		}
	}
}


Fat32::Fat32(IDE& ide) : mIde(ide) {

}

void Fat32::mount() {
	disk_mount(mIde);
}

/// Mounts a physical disk. It checks for a valid FAT32 file system in all
/// available disk partitions. All valid file system is dynamically allocated
/// and added to the system volumes
/// 
/// Note that this is the only functions referencing the `disk` parameter. All
/// further interactions happend will via the volume letter e.g. D: drive.
u8 Fat32::disk_mount(IDE& ide) {
	static fat_device_s fat_device;

	fat_device.mIde = &ide;

	// Verify that the storage device are present
	if (!ide.status()) return 0;
	
	// Initialize the hardware and protocols
	if (!ide.initialize()) return 0;
	
	// Read MBR sector at LBA address zero
	if (!ide.read(mount_buffer, 0, 1)) return 0;

	// Check the boot signature in the MBR
	if (fat_load16(mount_buffer + MBR_BOOT_SIG) != MBR_BOOT_SIG_VALUE) {
		return 0;
	}
	
	// Retrieve the partition info from all four partitions, thus avoiding 
	// multiple accesses to the MBR sector
	struct partition_s partitions[4];
	for (u8 i = 0; i < 4; i++) { 
		u32 offset = MBR_PARTITION + i * MBR_PARTITION_SIZE;
		
		partitions[i].lba = fat_load32(mount_buffer + offset + PAR_LBA);
		partitions[i].size = fat_load32(mount_buffer + offset + PAR_SIZE);
		partitions[i].type = mount_buffer[offset + PAR_TYPE];
		partitions[i].status = mount_buffer[offset + PAR_STATUS];
	}

	
	// Search for a valid FAT32 file systems on all valid paritions
	for (u8 i = 0; i < 4; i++) {
		if (partitions[i].lba) {
			if (!ide.read(mount_buffer, partitions[i].lba, 1)) {
				return 0;
			}
			
			// Check if the current partition contains a FAT32 file system
			if (fat_search(mount_buffer)) {
				
				// Allocate the file system structure
				struct volume_s* vol = (struct volume_s *)
					kmalloc(sizeof(struct volume_s));
				
				// Update FAT32 information
				vol->sector_size = fat_load16(mount_buffer + BPB_SECTOR_SIZE);
				vol->cluster_size = mount_buffer[BPB_CLUSTER_SIZE];
				vol->total_size = fat_load32(mount_buffer + BPB_TOT_SECT_32);
				vol->lba = partitions[i].lba;

				// Update FAT32 offsets that will be used by the driver
				vol->fsinfo_lba = partitions[i].lba + fat_load16(mount_buffer +
					BPB_32_FSINFO);
				vol->fat_lba = partitions[i].lba + fat_load16(mount_buffer + 
					BPB_RSVD_CNT);
				vol->data_lba = vol->fat_lba + (fat_load32(mount_buffer + 
					BPB_32_FAT_SIZE) * mount_buffer[BPB_NUM_FATS]);
				
				vol->root_lba = fat_clust_to_sect(vol, fat_load32(mount_buffer +
					BPB_32_ROOT_CLUST));
				vol->disk = ide.disk();
				
				// Sector zero will not exist in any file system. This forces 
				// the code to read the first block from the storage device
				vol->buffer_lba = 0;
				
				// Get the volume label
				fat_get_vol_label(ide, vol, vol->label);

				// Add the newly made volume to the list of system volumes
				fat_volume_add(vol);

			}
		}
	}

    struct volume_s* vol = volume_get('C');

	fat_device.mVol = vol;

    // Assign values to ufat.bpb
    uf.bpb.type = UFAT_TYPE_FAT32; // Assuming FAT32 since you're checking for it
    uf.bpb.log2_blocks_per_cluster = (unsigned int)log2f((float)vol->cluster_size);
    
    uf.bpb.fat_start = vol->fat_lba;
    uf.bpb.fat_size = fat_load32(mount_buffer + BPB_32_FAT_SIZE);
    uf.bpb.fat_count = mount_buffer[BPB_NUM_FATS];
    
    uf.bpb.cluster_start = vol->data_lba;
    uf.bpb.num_clusters = vol->total_size / (vol->cluster_size * vol->sector_size);
    
    uf.bpb.root_start = vol->root_lba;
    uf.bpb.root_size = 0; // For FAT32, root directory is not a fixed size
    uf.bpb.root_cluster = fat_load32(mount_buffer + BPB_32_ROOT_CLUST);

	device.data = (void*)&fat_device;
    device.log2_block_size = (uint8_t)(log2f((float)vol->sector_size));
    device.read = fat_device_read;
    device.write = fat_device_write;

	// u32 numblocks = vol->total_size / vol->sector_size;

	// ufat_mkfs(&device, numblocks);

    return 1;
}

/// Remove the volumes corresponding with a physical disk and delete the memory.
/// This function must be called before a storage device is unplugged, if not,
/// cached data may be lost. 
u8 Fat32::disk_eject(disk_e disk) {
	struct volume_s* vol = volume_get_first();
	
	while (vol != NULL) {
		
		// Remove all volumes which matches the `disk` number
		if (vol->disk == disk) {
			if (!fat_volume_remove(vol->letter)) {
				return 0;
			}
			kfree(vol);
		}
		vol = vol->next;
	}
	return 1;
}

/// Get the first volume in the system. If no volumes are present it return
/// NULL
struct volume_s* volume_get_first(void) {
	return volume_base;
}

/// Get a volume based on its letter
struct volume_s* volume_get(char letter) {
	
	struct volume_s* vol = volume_base;
	while (vol != NULL) {
		if (vol->letter == letter) {
			return vol;
		}
		vol = vol->next;
	}
	return NULL;
}

/// Set the volume label in the BPB SFN entry
fstatus volume_set_label(IDE& ide, struct volume_s* vol, const char* name, u8 length) {
	// Make a directory object pointing to the root directory
	struct dir_s dir;
	dir.sector = vol->root_lba;
	dir.rw_offset = 0;
	dir.cluster = fat_sect_to_clust(vol, dir.sector);
	
	while (1) {
		if (!fat_read(ide, vol, dir.sector)) {
			return FSTATUS_ERROR;
		}
		
		// Check if the attribute is volume label
		u8 attribute = vol->buffer[dir.rw_offset + SFN_ATTR];
		if (attribute & ATTR_VOL_LABEL) {
			
			// Check that it is not a LFN entry
			if ((attribute & ATTR_LFN) != ATTR_LFN) {
				char* src = (char *)(vol->buffer + dir.rw_offset);
				for (u8 i = 0; i < 11; i++) {
					// The volume label is padded with spaces
					if (i >= length) {
						*src++ = ' ';
					} else {
						*src++ = *name++;
					}
					vol->buffer_dirty = 1;
				}
				// Writes the buffer back to the storage device
				// TODO: Do I need this?
				fat_flush(ide, vol);
				return 1;
			}
		}
		// Get the next directory
		if (!fat_dir_get_next(ide, &dir)) {
			return 0;
		}
	}
}

/// Get the volume label
fstatus volume_get_label(IDE& ide, struct volume_s* vol, char* name) {
	// TODO: Hmm, this label is stored in the vol->label. Why fetch it two times
	return fat_get_vol_label(ide, vol, name);
}
