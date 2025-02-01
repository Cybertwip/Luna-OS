#ifdef __cplusplus
extern "C" {
#endif

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "sys/system.h"
#include "thread.h"

/**
 * @brief Represents a node in the list of threads managed by the scheduler.
 */
typedef struct thread_list {
    thread_t *thread;           /**< Pointer to the thread associated with this list node. */
    struct thread_list *next;   /**< Pointer to the next node in the thread list. */
} thread_list_t;

/**
 * @brief Initializes the scheduler with the given initial thread.
 * 
 * @param initial_thread Pointer to the thread to be set as the initial thread.
 */
void init_scheduler(thread_t *initial_thread);

/**
 * @brief Marks a thread as ready for execution.
 * 
 * @param t Pointer to the thread to mark as ready.
 */
void thread_is_ready(thread_t *t);

/**
 * @brief Marks a thread as not ready for execution.
 * 
 * @param t Pointer to the thread to mark as not ready.
 */
void thread_not_ready(thread_t *t);

/**
 * @brief Performs the scheduling operation to decide the next thread to execute.
 */
void schedule();

#endif /* SCHEDULER_H */

#ifdef __cplusplus
}
#endif
