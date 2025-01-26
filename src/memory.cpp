// kernel/memory.cpp
#include "memory.h"
#include <stddef.h> // for size_t
#include <stdint.h> // for uintptr_t

// Define a fixed-size memory pool
#define MEMORY_POOL_SIZE 1024 * 1024 // 1 MB memory pool
static char memory_pool[MEMORY_POOL_SIZE];
static size_t memory_used = 0;

// Simple free list node
struct FreeListNode {
    FreeListNode* next;
    size_t size;
};

static FreeListNode* free_list = nullptr;

// Kernel memory allocation function
void* kmalloc(size_t size) {
    if (size == 0) {
        return nullptr; // Cannot allocate 0 bytes
    }

    // Align size to 8 bytes for simplicity
    size = (size + 7) & ~7;

    // Check the free list for a suitable block
    FreeListNode** node = &free_list;
    while (*node) {
        if ((*node)->size >= size) {
            FreeListNode* found = *node;
            *node = found->next; // Remove from free list
            return reinterpret_cast<void*>(found + 1); // Return the block after the node
        }
        node = &(*node)->next;
    }

    // If no block found in the free list, allocate from the memory pool
    if (memory_used + size > MEMORY_POOL_SIZE) {
        return nullptr; // Out of memory
    }

    void* ptr = &memory_pool[memory_used];
    memory_used += size;
    return ptr;
}

// Kernel memory deallocation function
void kfree(void* ptr) {
    if (!ptr) {
        return; // Do nothing for nullptr
    }

    // Get the FreeListNode from the block
    FreeListNode* node = reinterpret_cast<FreeListNode*>(ptr) - 1;

    // Add the block to the free list
    node->next = free_list;
    free_list = node;
}