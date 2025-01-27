// kernel/memory.cpp (updated)
#include "memory.h"
#include <stddef.h>
#include <stdint.h>
#include <std/spinlock.h>

// Memory pool configuration
#define MEMORY_POOL_SIZE (1024 * 1024 * 1024) // 1024MB initial pool
static char memory_pool[MEMORY_POOL_SIZE];
static size_t memory_used = 0;

// Define heap symbols for picolibc's sbrk
extern "C" {
    uintptr_t __heap_start = reinterpret_cast<uintptr_t>(&memory_pool[0]);
    uintptr_t __heap_end = __heap_start + MEMORY_POOL_SIZE;
}

// Free list structure with size and next pointer
struct FreeListNode {
    size_t size;
    FreeListNode* next;
};

static FreeListNode* free_list = nullptr;
static spinlock_t alloc_lock; // Spinlock for thread safety

// Helper function to coalesce adjacent blocks
static void coalesce_blocks(FreeListNode* block) {
    FreeListNode* current = free_list;
    FreeListNode* prev = nullptr;

    while (current) {
        // Check adjacent blocks
        uintptr_t current_end = reinterpret_cast<uintptr_t>(current) + current->size;
        uintptr_t block_start = reinterpret_cast<uintptr_t>(block);
        
        if (current_end == block_start) {
            // Merge current with block
            current->size += block->size;
            block = current;
            if (prev) prev->next = current->next;
            else free_list = current->next;
            current = block;
        }
        else if (reinterpret_cast<uintptr_t>(current) == 
                (reinterpret_cast<uintptr_t>(block) + block->size)) {
            // Merge block with current
            block->size += current->size;
            block->next = current->next;
            if (prev) prev->next = block;
            else free_list = block;
            current = block;
        }
        
        prev = current;
        current = current->next;
    }
}

void* kmalloc(size_t size) {
    if (size == 0) return nullptr;

    spin_lock(&alloc_lock);
    
    // Calculate aligned size with metadata
    size = ALIGN(size + sizeof(FreeListNode));
    
    // Search free list
    FreeListNode** node = &free_list;
    while (*node) {
        if ((*node)->size >= size) {
            // Found suitable block
            FreeListNode* found = *node;
            size_t remaining = found->size - size;
            
            if (remaining >= sizeof(FreeListNode) + ALIGNMENT) {
                // Split block
                FreeListNode* new_node = reinterpret_cast<FreeListNode*>(
                    reinterpret_cast<char*>(found) + size
                );
                new_node->size = remaining;
                new_node->next = found->next;
                *node = new_node;
            } else {
                // Take entire block
                *node = found->next;
            }
            
            found->size = size;
            spin_unlock(&alloc_lock);
            return reinterpret_cast<void*>(found + 1);
        }
        node = &(*node)->next;
    }

    // Allocate from pool
    size_t required = size;
    if (memory_used + required > MEMORY_POOL_SIZE) {
        spin_unlock(&alloc_lock);
        return nullptr; // Out of memory (should expand pool here)
    }

    FreeListNode* new_block = reinterpret_cast<FreeListNode*>(
        &memory_pool[memory_used]
    );
    new_block->size = size;
    memory_used += size;
    
    spin_unlock(&alloc_lock);
    return reinterpret_cast<void*>(new_block + 1);
}

void kfree(void* ptr) {
    if (!ptr) return;

    spin_lock(&alloc_lock);
    
    FreeListNode* block = reinterpret_cast<FreeListNode*>(ptr) - 1;
    
    // Add to free list and coalesce
    block->next = free_list;
    free_list = block;
    
    coalesce_blocks(block);
    
    spin_unlock(&alloc_lock);
}