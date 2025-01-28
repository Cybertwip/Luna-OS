#include "sys/scheduler.h"
#include "mm/kmalloc.h"

// Global variables for the scheduler
thread_list_t *ready_queue = 0;     // Points to the queue of ready threads
thread_list_t *current_thread = 0; // Points to the currently running thread

/**
 * @brief Initializes the scheduler with the initial thread.
 *
 * @param initial_thread A pointer to the thread structure for the initial thread.
 */
void init_scheduler(thread_t *initial_thread)
{
    // Allocate memory for the current thread node
    current_thread = (thread_list_t *)kmalloc(sizeof(thread_list_t));
    current_thread->thread = initial_thread;
    current_thread->next = 0; // No next thread initially
    ready_queue = 0;          // Initialize the ready queue as empty
}

/**
 * @brief Adds a thread to the ready queue, marking it as ready to run.
 *
 * @param t A pointer to the thread structure for the thread to add.
 */
void thread_is_ready(thread_t *t)
{
    // Allocate memory for the new thread node
    thread_list_t *item = (thread_list_t *)kmalloc(sizeof(thread_list_t));
    item->thread = t;
    item->next = 0;

    if (!ready_queue)
    {
        // If the ready queue is empty, set the new thread as the head
        ready_queue = item;
    }
    else
    {
        // Traverse to the end of the ready queue
        thread_list_t *iterator = ready_queue;
        while (iterator->next)
        {
            iterator = iterator->next;
        }

        // Add the new thread to the end of the queue
        iterator->next = item;
    }
}

/**
 * @brief Removes a thread from the ready queue, marking it as not ready to run.
 *
 * @param t A pointer to the thread structure for the thread to remove.
 */
void thread_not_ready(thread_t *t)
{
    // Pointer to traverse the ready queue
    thread_list_t *iterator = ready_queue;

    // Special case: if the thread is at the head of the queue
    if (iterator && iterator->thread == t)
    {
        ready_queue = iterator->next; // Update the head of the queue
        kfree(iterator);              // Free the memory of the removed node
        return;
    }

    // Traverse the queue to find the thread
    while (iterator && iterator->next)
    {
        if (iterator->next->thread == t)
        {
            // Found the thread, remove it
            thread_list_t *tmp = iterator->next;
            iterator->next = tmp->next; // Update the next pointer
            kfree(tmp);                 // Free the memory of the removed node
            return;
        }
        iterator = iterator->next;
    }
}

/**
 * @brief Performs a context switch to the next thread in the ready queue.
 *
 * Moves the currently running thread to the end of the ready queue
 * and switches to the thread at the head of the queue.
 */
void schedule()
{
    // If there are no threads in the ready queue, return
    if (!ready_queue) return;

    // Traverse to the end of the ready queue
    thread_list_t *iterator = ready_queue;
    while (iterator->next)
    {
        iterator = iterator->next;
    }

    // Move the current thread to the end of the ready queue
    iterator->next = current_thread;
    current_thread->next = 0; // Current thread becomes the last node

    // Take the first thread from the ready queue
    thread_list_t *new_thread = ready_queue;
    ready_queue = ready_queue->next; // Update the head of the queue

    // Switch to the new thread
    switch_thread(new_thread);
}
