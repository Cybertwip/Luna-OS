#include "spinlock.h"

// Memory barrier to prevent compiler reordering
#define MEMORY_BARRIER() __asm__ volatile ("" ::: "memory")

// Initialize the spinlock
void spinlock_init(spinlock_t *lock) {
    lock->lock = 0; // 0 means unlocked
}

// Acquire the spinlock with exponential backoff
void spin_lock(spinlock_t *lock) {
    int backoff = 1;
    while (1) {
        // Try to acquire the lock
        if (__sync_bool_compare_and_swap(&lock->lock, 0, 1)) {
            break; // Lock acquired
        }
        // Exponential backoff
        for (int i = 0; i < backoff; ++i) {
            MEMORY_BARRIER();
        }
        backoff *= 2; // Increase backoff exponentially
    }
}

// Release the spinlock
void spin_unlock(spinlock_t *lock) {
    // Memory barrier to ensure all previous writes are visible
    MEMORY_BARRIER();
    lock->lock = 0; // Release the lock
}

// Try to acquire the spinlock (non-blocking)
int spin_trylock(spinlock_t *lock) {
    return __sync_bool_compare_and_swap(&lock->lock, 0, 1);
}