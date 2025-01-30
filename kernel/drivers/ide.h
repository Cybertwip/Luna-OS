#pragma once
#include <stdint.h>

unsigned int ide_read_blocks(unsigned int minor, uint32_t block,
    unsigned int nblocks, void *buffer);

unsigned int ide_write_blocks(unsigned int minor, uint32_t block,
    unsigned int nblocks, void *buffer);
    
extern void init_ide(void);
