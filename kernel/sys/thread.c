#include "thread.h"
#include "mm/kmalloc.h"
#include "sys/scheduler.h"

// Current running thread.
static thread_t *current_thread;

// Global variable to assign unique thread IDs.
uint32_t next_tid = 0;

// Forward declaration for thread exit function.
void thread_exit();

// External assembly function to create a thread.
extern void _create_thread(int (*fn)(void*), void *arg, uint32_t *stack, thread_t *thread);

/**
 * @brief Initializes threading by creating the first thread.
 * 
 * Allocates memory for a new thread structure, assigns it a unique ID, and
 * sets it as the current thread.
 * 
 * @return A pointer to the initialized thread structure.
 */
thread_t *init_threading() {
    // Allocate memory for the initial thread structure.
    thread_t *thread = kmalloc(sizeof(thread_t));
    thread->id = next_tid++;  // Assign a unique thread ID.

    // Set the current thread to the newly created thread.
    current_thread = thread;

    return thread;
}

/**
 * @brief Creates a new thread.
 * 
 * Allocates memory for a thread structure, sets up the stack, and initializes
 * the thread's state. The thread is marked as ready to run.
 * 
 * @param fn    The function the thread will execute.
 * @param arg   The argument passed to the thread function.
 * @param stack Pointer to the stack memory allocated for the thread.
 * @return A pointer to the newly created thread structure.
 */
thread_t *create_thread(int (*fn)(void*), void *arg, uint32_t *stack) {
    // Allocate memory for the new thread structure.
    thread_t *thread = kmalloc(sizeof(thread_t));
    memset(thread, 0, sizeof(thread_t));  // Clear the memory for initialization.
    thread->id = next_tid++;  // Assign a unique thread ID.

    // Set up the thread's stack in reverse order: arguments, return address, function pointer.
    *--stack = (uint32_t)arg;       // Argument for the thread function.
    *--stack = (uint32_t)&thread_exit;  // Return address (thread exit function).
    *--stack = (uint32_t)fn;        // Thread function entry point.

    // Set the thread's stack pointer and initial register values.
    thread->esp = (uint32_t)stack;  // Stack pointer.
    thread->ebp = 0;                // Base pointer (unused here).
    thread->eflags = 0x200;         // Interrupts enabled.

    // Mark the thread as ready to be scheduled.
    thread_is_ready(thread);

    return thread;
}

/**
 * @brief Terminates the current thread.
 * 
 * This function is called when a thread finishes execution. It prints the
 * exit value of the thread and enters an infinite loop to prevent further
 * execution.
 */
void thread_exit() {
    // Retrieve the exit value from the eax register.
    register uint32_t val __asm__("eax");

    // Print the exit value of the thread.
    printk("Thread exited with value %d\n", val);

    // Enter an infinite loop to prevent the thread from executing further.
    for (;;) ;
}
