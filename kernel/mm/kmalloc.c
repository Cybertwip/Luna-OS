#include "sys/system.h"
#include "paging.h"
#include "heap.h"
#include "mm/kmalloc.h"

#include <stdint.h>
#include <stddef.h>

// External references
extern uint32_t __end;                          // End of the kernel (defined in the linker script)
extern struct vm_heap *kernel_heap;              // Pointer to the kernel's heap (defined in heap.c)
extern struct vm_page_directory *kernel_directory; // Pointer to the kernel's page directory (defined in paging.c)

uint32_t placement_address = (uint32_t)&__end;   // Address for placement-based allocation (initializes to the end of the kernel)

/* Internal allocation routine */
static void * _kmalloc(size_t len, uint32_t *phys, uint32_t flags);

/* Bitmask flags for _kmalloc() */
#define M_ALIGNED  0x1  // Flag for requesting aligned memory
#define M_ZERO      0x2  // Flag for requesting zeroed memory


/**
 * Allocates memory from the heap.
 * 
 * @param len The size of memory to allocate.
 * @return A pointer to the allocated memory.
 */
void * kmalloc(size_t len)
{
    return _kmalloc(len, NULL, 0);
}

/**
 * Allocates zero-initialized memory from the heap.
 * 
 * @param len The size of memory to allocate.
 * @return A pointer to the allocated memory (zeroed).
 */
void * kmalloc0(size_t len)
{
    return _kmalloc(len, NULL, M_ZERO);
}

/**
 * Allocates aligned memory from the heap.
 * 
 * @param len The size of memory to allocate.
 * @return A pointer to the allocated memory (aligned).
 */
void * kmalloc_a(size_t len)
{
    return _kmalloc(len, NULL, M_ALIGNED);
}

/**
 * Allocates aligned and zero-initialized memory from the heap.
 * 
 * @param len The size of memory to allocate.
 * @return A pointer to the allocated memory (aligned and zeroed).
 */
void * kmalloc0_a(size_t len)
{
    return _kmalloc(len, NULL, (M_ALIGNED | M_ZERO));
}

/**
 * Allocates memory and returns its physical address.
 * 
 * @param len The size of memory to allocate.
 * @param phys A pointer to store the physical address of the allocated memory.
 * @return A pointer to the allocated memory.
 */
void * kmalloc_p(size_t len, uint32_t *phys)
{
    return _kmalloc(len, phys, 0);
}

/**
 * Allocates zero-initialized memory and returns its physical address.
 * 
 * @param len The size of memory to allocate.
 * @param phys A pointer to store the physical address of the allocated memory.
 * @return A pointer to the allocated memory (zeroed).
 */
void * kmalloc0_p(size_t len, uint32_t *phys)
{
    return _kmalloc(len, phys, M_ZERO);
}

/**
 * Allocates aligned memory and returns its physical address.
 * 
 * @param len The size of memory to allocate.
 * @param phys A pointer to store the physical address of the allocated memory.
 * @return A pointer to the allocated memory (aligned).
 */
void * kmalloc_ap(size_t len, uint32_t *phys)
{
    return _kmalloc(len, phys, M_ALIGNED);
}

/**
 * Allocates aligned and zero-initialized memory and returns its physical address.
 * 
 * @param len The size of memory to allocate.
 * @param phys A pointer to store the physical address of the allocated memory.
 * @return A pointer to the allocated memory (aligned and zeroed).
 */
void * kmalloc0_ap(size_t len, uint32_t *phys)
{
    return _kmalloc(len, phys, (M_ALIGNED | M_ZERO));
}

/**
 * Internal memory allocation function.
 * 
 * This function allocates memory either from a static placement address (if no kernel heap is available)
 * or from the kernel heap, and optionally returns the physical address and aligns the memory if needed.
 * 
 * @param len The size of memory to allocate.
 * @param phys A pointer to store the physical address of the allocated memory (can be NULL).
 * @param flags Flags for additional memory properties (alignment and zero initialization).
 * @return A pointer to the allocated memory.
 */
static void * _kmalloc(size_t len, uint32_t *phys, uint32_t flags)
{
    void *addr = NULL;

    // If no kernel heap is available, allocate from the placement address
    if (kernel_heap == NULL) {
        // Ensure the address is aligned if requested
        if ((flags & M_ALIGNED) && (placement_address & 0xFFFFF000)) {
            placement_address = (placement_address & 0xFFFFF000) + 0x1000; // Align to next 4KB boundary
        }

        // Return physical address if requested
        if (phys != NULL)
            *phys = placement_address;

        // Update the placement address and return the allocated memory
        placement_address += len;
        addr = (void *)(placement_address - len);
    } else {
        // Otherwise, allocate memory from the kernel heap
        addr = halloc(len, (flags & M_ALIGNED), kernel_heap);

        // Return the physical address if requested
        if (phys != NULL) {
            struct vm_page *page = get_page((uint32_t)addr, 0, kernel_directory);
            *phys = page->p_frame * 0x1000 + ((uint32_t)addr & 0xFFF);
        }
    }

    // Zero the memory if the M_ZERO flag is set
    if (flags & M_ZERO)
        bzero(addr, len);

    return addr;
}

/**
 * Frees the allocated memory back to the heap.
 * 
 * @param ptr A pointer to the memory to free.
 */
void kfree(void *ptr)
{
    hfree(ptr, kernel_heap);
}

size_t get_block_size(void *ptr) {
    if (ptr == NULL) {
        return 0;
    }

    // The size is stored in a header before the allocated memory
    size_t *size_ptr = (size_t *)ptr - 1;
    return *size_ptr;
}


void *luna_malloc(size_t size) {
    return _kmalloc(size, NULL, 0);
}

void *luna_calloc(size_t num, size_t size) {
    // Calculate the total size to allocate
    size_t total_size = num * size;

    // Check for overflow in the multiplication
    if (size != 0 && total_size / size != num) {
        return NULL; // Overflow occurred
    }

    // Allocate memory using _kmalloc with the M_ZERO flag to zero the memory
    void *ptr = _kmalloc(total_size, NULL, M_ZERO);

    return ptr;
}

void *luna_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        // If ptr is NULL, realloc behaves like malloc
        return malloc(size);
    }

    if (size == 0) {
        // If size is zero, realloc behaves like free
        kfree(ptr);
        return NULL;
    }

    // Allocate a new block of memory
    void *new_ptr = malloc(size);
    if (new_ptr == NULL) {
        return NULL; // Allocation failed
    }

    // Copy the data from the old block to the new block
    // Determine the size of the old block (this requires tracking the size of allocated blocks)
    size_t old_size = get_block_size(ptr); // This function needs to be implemented
    memcpy(new_ptr, ptr, old_size < size ? old_size : size);

    // Free the old block
    kfree(ptr);

    return new_ptr;
}

void luna_free(void *ptr) {
    if (ptr != NULL) {
        kfree(ptr);
    }
}