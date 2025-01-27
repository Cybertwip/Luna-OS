#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h> // For uint32_t

typedef struct {
    volatile uint32_t lock; // Lock variable (0 = unlocked, 1 = locked)
} spinlock_t;

// Initialize a spinlock
void spinlock_init(spinlock_t *lock);

// Acquire a spinlock
void spin_lock(spinlock_t *lock);

// Release a spinlock
void spin_unlock(spinlock_t *lock);

// Try to acquire a spinlock (non-blocking)
int spin_trylock(spinlock_t *lock);

#endif // SPINLOCK_H