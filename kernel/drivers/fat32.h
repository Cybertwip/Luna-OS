#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stdbool.h>

#define SECTOR_SIZE 512
#define CLUSTER_FREE 0x00000000
#define CLUSTER_BAD  0x0FFFFFF7
#define CLUSTER_EOF  0x0FFFFFF8
#define CLUSTER_MASK 0x0FFFFFFF

typedef struct {
    uint32_t bytes_per_sector;
    uint32_t sectors_per_cluster;
    uint32_t reserved_sectors;
    uint32_t num_fats;
    uint32_t sectors_per_fat;
    uint32_t root_cluster;
    uint32_t fat_start;
    uint32_t data_start;
    uint32_t total_clusters;
} FAT32_Volume;

typedef struct {
    char name[11];
    uint8_t attributes;
    uint32_t size;
    uint32_t start_cluster;
    uint32_t current_cluster;
    uint32_t position;
} FAT32_File;

// Volume operations
bool FAT32_Init(uint32_t lba_start);
FAT32_Volume* FAT32_GetVolume();

// File operations
bool FAT32_Open(FAT32_File* file, const char* path);
uint32_t FAT32_Read(FAT32_File* file, void* buffer, uint32_t size);
bool FAT32_Close(FAT32_File* file);

// Directory operations
bool FAT32_ReadDir(FAT32_File* dir, FAT32_File* entry);

// Utility functions
uint32_t FAT32_ClusterToLBA(uint32_t cluster);

#endif