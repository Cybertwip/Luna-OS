#include <mm/sbrk.h>
#include <mm/paging.h>  // For PAGE_SIZE and page management functions
#include <sys/errno.h>
#include <stddef.h>

static void *kernel_brk = NULL;

// Initialize the kernel's break address
void sbrk_init(void *start) {
    kernel_brk = start;
}

// Adjust the kernel's program break
void *sbrk(intptr_t increment) {
    if (kernel_brk == NULL) {
        // sbrk not initialized
        errno = EINVAL;
        return (void *)-1;
    }

    if (increment == 0) {
        return kernel_brk; // Current break
    }

    void *old_brk = kernel_brk;
    void *new_brk = (char *)old_brk + increment;

    // Handle positive increments (expand heap)
    if (increment > 0) {
        uintptr_t old_end = (uintptr_t)old_brk;
        uintptr_t new_end = (uintptr_t)new_brk;

        // Align addresses to page boundaries
        uintptr_t page_start = (old_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        uintptr_t page_end = new_end & ~(PAGE_SIZE - 1);

        // Allocate and map new pages if needed
        for (uintptr_t page = page_start; page < page_end; page += PAGE_SIZE) {
            struct vm_page *pg = get_page(page, 1, kernel_directory);
            if (!pg) {
                errno = ENOMEM;
                return (void *)-1;
            }
            if (!pg->p_present) {
                alloc_frame(pg, 1, 1); // Allocate frame for kernel, writable
                if (!pg->p_present) { // Check if allocation succeeded
                    errno = ENOMEM;
                    return (void *)-1;
                }
            }
        }
    }

    // Update the kernel break (no shrink handling for simplicity)
    kernel_brk = new_brk;
    return old_brk;
}