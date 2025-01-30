#include "fat32.h"
#include "ide.h"
#include "string.h"
#include "sys/endian.h"
#include "sys/system.h"

static FAT32_Volume vol;

#pragma pack(push, 1)
typedef struct {
    uint8_t jmp[3];
    char oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t nt_flags;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
} FAT32_BPB;

typedef struct {
    char name[8];
    char ext[3];
    uint8_t attributes;
    uint8_t nt_reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t cluster_low;
    uint32_t size;
} FAT32_DirEntry;
#pragma pack(pop)

static uint8_t fat_cache[SECTOR_SIZE];
static uint32_t current_fat_sector = 0xFFFFFFFF;

bool FAT32_Init(uint32_t lba_start) {
    uint8_t sector[SECTOR_SIZE];
    ide_read_blocks(0, lba_start, 1, sector);

    FAT32_BPB* bpb = (FAT32_BPB*)sector;
    
    if (memcmp(bpb->fs_type, "FAT32   ", 8) != 0) {
        printk("Not a FAT32 filesystem\n");
        return false;
    }

    vol.bytes_per_sector = bpb->bytes_per_sector;
    vol.sectors_per_cluster = bpb->sectors_per_cluster;
    vol.reserved_sectors = bpb->reserved_sectors;
    vol.num_fats = bpb->num_fats;
    vol.sectors_per_fat = bpb->sectors_per_fat;
    vol.root_cluster = bpb->root_cluster;
    
    vol.fat_start = lba_start + vol.reserved_sectors;
    vol.data_start = vol.fat_start + (vol.num_fats * vol.sectors_per_fat);
    vol.total_clusters = (vol.sectors_per_fat * SECTOR_SIZE * 8) / 32;

    return true;
}

uint32_t FAT32_ClusterToLBA(uint32_t cluster) {
    return vol.data_start + ((cluster - 2) * vol.sectors_per_cluster);
}

static uint32_t read_fat_entry(uint32_t cluster) {
    uint32_t fat_sector = vol.fat_start + (cluster * 4 / SECTOR_SIZE);
    uint32_t offset = (cluster * 4) % SECTOR_SIZE;

    if (current_fat_sector != fat_sector) {
        ide_read_blocks(0, fat_sector, 1, fat_cache);
        current_fat_sector = fat_sector;
    }

    return *(uint32_t*)(fat_cache + offset) & CLUSTER_MASK;
}

bool FAT32_Open(FAT32_File* file, const char* path) {
    // Simplified root directory opening
    if (strcmp(path, "/") == 0) {
        file->start_cluster = vol.root_cluster;
        file->current_cluster = vol.root_cluster;
        file->position = 0;
        return true;
    }
    return false;
}

uint32_t FAT32_Read(FAT32_File* file, void* buffer, uint32_t size) {
    uint8_t* buf = (uint8_t*)buffer;
    uint32_t bytes_read = 0;
    
    while (bytes_read < size) {
        uint32_t sector_in_cluster = (file->position / SECTOR_SIZE) % vol.sectors_per_cluster;
        uint32_t cluster = file->current_cluster;
        
        if (cluster < 2 || cluster >= vol.total_clusters) break;
        
        uint32_t lba = FAT32_ClusterToLBA(cluster) + sector_in_cluster;
        uint32_t offset = file->position % SECTOR_SIZE;
        uint32_t to_read = SECTOR_SIZE - offset;
        
        if (to_read > (size - bytes_read))
            to_read = size - bytes_read;

        uint8_t sector[SECTOR_SIZE];
        ide_read_blocks(0, lba, 1, sector);
        memcpy(buf + bytes_read, sector + offset, to_read);
        
        bytes_read += to_read;
        file->position += to_read;

        if ((file->position / SECTOR_SIZE) % vol.sectors_per_cluster == 0) {
            file->current_cluster = read_fat_entry(file->current_cluster);
        }
    }
    
    return bytes_read;
}

bool FAT32_ReadDir(FAT32_File* dir, FAT32_File* entry) {
    FAT32_DirEntry dirent;
    
    while (1) {
        if (FAT32_Read(dir, &dirent, sizeof(FAT32_DirEntry)) != sizeof(FAT32_DirEntry))
            return false;

        if (dirent.name[0] == 0x00) return false;  // End of directory
        if (dirent.name[0] == 0xE5) continue;      // Deleted entry
        if (dirent.attributes == 0x0F) continue;   // Long filename entry

        // Convert little-endian fields to host byte order
        uint32_t size = le32toh(dirent.size);
        uint16_t cluster_high = le32toh(dirent.cluster_high);
        uint16_t cluster_low = le32toh(dirent.cluster_low);

        // Convert 8.3 name to "name.ext"
        memset(entry->name, 0, sizeof(entry->name));
        char *name_ptr = entry->name;
        
        // Copy name part (up to 8 characters, trim trailing spaces)
        int name_len = 0;
        while (name_len < 8 && dirent.name[name_len] != ' ') {
            *name_ptr++ = dirent.name[name_len++];
        }
        
        // Copy extension part (up to 3 characters, trim trailing spaces)
        if (dirent.ext[0] != ' ') {
            *name_ptr++ = '.';
            int ext_len = 0;
            while (ext_len < 3 && dirent.ext[ext_len] != ' ') {
                *name_ptr++ = dirent.ext[ext_len++];
            }
        }
        
        *name_ptr = '\0'; // Null-terminate the string

        entry->size = size;
        entry->start_cluster = ((uint32_t)cluster_high << 16) | cluster_low;
        entry->attributes = dirent.attributes;
        
        
        return true;
    }
}


FAT32_Volume* FAT32_GetVolume() {
    return &vol;
}