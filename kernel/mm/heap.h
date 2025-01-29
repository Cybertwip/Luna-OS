#ifndef INCLUDE_HEAP_H
#define INCLUDE_HEAP_H

#include "sys/system.h"
#include "sorted_array.h"

// Memory constants for heap management
#define VM_KERN_HEAP_START           0xC0000000   // Start address for the kernel heap
#define VM_KERN_HEAP_INITIAL_SIZE    0x800000     // Initial size of the kernel heap

#define VM_HEAP_INDEX_SIZE           0x20000      // Size of the heap index
#define VM_HEAP_HDR_MAGIC            0x123890AB   // Magic number used for block header identification
#define VM_HEAP_FTR_MAGIC            0xBA098321   // Magic number used for block footer identification
#define VM_HEAP_MIN_SIZE             0x70000      // Minimum size of the heap

/*
 * Heap Block Structure Definitions:
 * These structures are used to manage the heap's allocated and free memory regions.
 */

// Header structure for each block in the heap
struct vm_heap_header {
    uint32_t hh_magic;  // Magic number (VM_HEAP_HDR_MAGIC) for error checking and identification
    int hh_is_hole;     // Flag: 1 if this is a hole (free block), 0 if this is an allocated block
    size_t hh_size;     // Size of the block, including the footer
};

// Footer structure for each block in the heap
struct vm_heap_footer {
    uint32_t hf_magic;           // Magic number (VM_HEAP_FTR_MAGIC) for error checking
    struct vm_heap_header *hf_header; // Pointer to the corresponding block header
};

// Main heap structure to manage the heap's state
struct vm_heap {
    struct sorted_array h_index;     // Index of free memory blocks (sorted array for quick allocation)
    uint32_t h_addr_start;           // Start address of the heap's allocated space
    uint32_t h_addr_end;             // End address of the heap's allocated space
    uint32_t h_addr_max;             // Maximum address the heap can expand to
    int h_su;                        // Supervisor-only mapping flag (1: supervisor-only pages, 0: normal pages)
    int h_ro;                        // Read-only mapping flag (1: read-only pages, 0: writable pages)
};

/*
 * Function Prototypes:
 * These functions manage the heap memory allocation and initialization.
 */

/**
 * Allocates and initializes a new heap structure with the specified parameters.
 * 
 * @param start The starting address of the heap.
 * @param end The ending address of the heap.
 * @param max The maximum address the heap can expand to.
 * @param su Flag indicating if extra pages should be mapped as supervisor-only (1: supervisor-only, 0: normal).
 * @param ro Flag indicating if extra pages should be mapped as read-only (1: read-only, 0: writable).
 * 
 * @return A pointer to the newly created heap structure.
 */
struct vm_heap *new_heap(uint32_t start, uint32_t end, uint32_t max, int su, int ro);

/**
 * Initializes an existing heap structure with the specified parameters.
 * 
 * @param heap A pointer to the heap structure to be initialized.
 * @param start The starting address of the heap.
 * @param end The ending address of the heap.
 * @param max The maximum address the heap can expand to.
 * @param su Flag indicating if extra pages should be mapped as supervisor-only (1: supervisor-only, 0: normal).
 * @param ro Flag indicating if extra pages should be mapped as read-only (1: read-only, 0: writable).
 * 
 * @return A pointer to the initialized heap structure.
 */
struct vm_heap *init_heap(struct vm_heap *heap, uint32_t start, uint32_t end, uint32_t max, int su, int ro);

/**
 * Allocates a contiguous block of memory of the specified size.
 * 
 * If `page_align` is set to 1, the block will be aligned to a page boundary.
 * 
 * @param size The size of the memory block to allocate.
 * @param page_align Flag indicating if the block should be page-aligned (1: align, 0: no alignment).
 * @param heap A pointer to the heap structure where the memory should be allocated.
 * 
 * @return A pointer to the allocated memory block.
 */
void *halloc(uint32_t size, int page_align, struct vm_heap *heap);

/**
 * Frees a block of memory previously allocated using `alloc`.
 * 
 * @param p A pointer to the block of memory to free.
 * @param heap A pointer to the heap structure to which the memory belongs.
 */
void hfree(void *p, struct vm_heap *heap);

#endif /* INCLUDE_HEAP_H */
