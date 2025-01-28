#ifndef THREAD_H
#define THREAD_H

#include "sys/system.h"

/**
 * @struct thread_t
 * @brief Represents the context of a thread in the system.
 *
 * This structure holds the state of a thread, including its CPU registers 
 * and a unique identifier.
 */
typedef struct {
    uint32_t esp;    ///< Stack pointer
    uint32_t ebp;    ///< Base pointer
    uint32_t ebx;    ///< General-purpose register
    uint32_t esi;    ///< General-purpose register
    uint32_t edi;    ///< General-purpose register
    uint32_t eflags; ///< CPU flags register
    uint32_t id;     ///< Unique thread identifier
} thread_t;

/**
 * @brief Initializes the threading system.
 *
 * Sets up the environment for threading and returns the initial thread structure.
 * 
 * @return A pointer to the initial thread structure.
 */
thread_t *init_threading();

/**
 * @brief Creates a new thread.
 *
 * Allocates resources and initializes the context for a new thread.
 *
 * @param fn Pointer to the thread function to execute.
 * @param arg Argument to pass to the thread function.
 * @param stack Pointer to the pre-allocated stack memory.
 * @return A pointer to the newly created thread structure.
 */
thread_t *create_thread(int (*fn)(void*), void *arg, uint32_t *stack);

/**
 * @struct thread_list
 * @brief Represents a linked list or array of threads.
 *
 * This is likely defined elsewhere in the codebase.
 */
struct thread_list;

/**
 * @brief Switches execution to the next thread.
 *
 * Performs a context switch to the specified next thread.
 *
 * @param next Pointer to the next thread's list structure.
 */
void switch_thread(struct thread_list *next);

#endif // THREAD_H
