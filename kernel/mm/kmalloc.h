#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdint.h>
#include <stddef.h>

/*
 * This header file contains the declarations for memory allocation routines.
 * All `kmalloc0*` functions ensure that the allocated memory is initialized 
 * with 0x0 (zeroed out).
 */

/**
 * @brief Allocates a block of memory of the specified length.
 * 
 * @param len The size of the memory block to allocate, in bytes.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
void *kmalloc(size_t len);  /* classic malloc */

/**
 * @brief Allocates a block of memory of the specified length and initializes it to 0.
 * 
 * @param len The size of the memory block to allocate, in bytes.
 * @return A pointer to the allocated (zeroed) memory block, or NULL if the allocation fails.
 */
void *kmalloc0(size_t len);

/**
 * @brief Allocates a block of memory of the specified length, with page alignment.
 * 
 * @param len The size of the memory block to allocate, in bytes.
 * @return A pointer to the allocated memory block, aligned to a page boundary, or NULL if the allocation fails.
 */
void *kmalloc_a(size_t len);  /* page aligned malloc */

/**
 * @brief Allocates a block of memory of the specified length, with page alignment,
 *        and initializes it to 0.
 * 
 * @param len The size of the memory block to allocate, in bytes.
 * @return A pointer to the allocated (zeroed) memory block, aligned to a page boundary, or NULL if the allocation fails.
 */
void *kmalloc0_a(size_t len);

/**
 * @brief Allocates a block of memory of the specified length and returns its physical address.
 * 
 * @param len The size of the memory block to allocate, in bytes.
 * @param phys A pointer to store the physical address of the allocated memory.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
void *kmalloc_p(size_t len, uint32_t *phys);  /* physical address */

/**
 * @brief Allocates a block of memory of the specified length, initializes it to 0,
 *        and returns its physical address.
 * 
 * @param len The size of the memory block to allocate, in bytes.
 * @param phys A pointer to store the physical address of the allocated memory.
 * @return A pointer to the allocated (zeroed) memory block, or NULL if the allocation fails.
 */
void *kmalloc0_p(size_t len, uint32_t *phys);

/**
 * @brief Allocates a block of memory of the specified length, with page alignment,
 *        and returns its physical address.
 * 
 * @param len The size of the memory block to allocate, in bytes.
 * @param phys A pointer to store the physical address of the allocated memory.
 * @return A pointer to the allocated memory block, aligned to a page boundary, or NULL if the allocation fails.
 */
void *kmalloc_ap(size_t len, uint32_t *phys);  /* page aligned and physical address */

/**
 * @brief Allocates a block of memory of the specified length, with page alignment,
 *        initializes it to 0, and returns its physical address.
 * 
 * @param len The size of the memory block to allocate, in bytes.
 * @param phys A pointer to store the physical address of the allocated memory.
 * @return A pointer to the allocated (zeroed) memory block, aligned to a page boundary, or NULL if the allocation fails.
 */
void *kmalloc0_ap(size_t len, uint32_t *phys);

/**
 * @brief Frees a previously allocated block of memory.
 * 
 * @param ptr A pointer to the memory block to free.
 */
void kfree(void *ptr);

#endif /* KMALLOC_H */
