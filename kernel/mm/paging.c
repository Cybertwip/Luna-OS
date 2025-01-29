#include "paging.h"
#include "heap.h"
#include "kmalloc.h"

/* A bitset of frames - used or free. */
uint32_t *frames;
uint32_t nframes;

/* Defined in kmalloc.c */
extern uint32_t placement_address;

/* The kernel's page directory */
struct vm_page_directory *kernel_directory;
/* The current page directory */
struct vm_page_directory *current_directory;

/* Macros used in the bitset algorithms. */
#define INDEX_FROM_BIT(a)  ((a) / (8 * 4))          /* Convert frame number to bitset index */
#define OFFSET_FROM_BIT(a) ((a) % (8 * 4))          /* Calculate bit position within the bitset */

/**
 * @brief Sets a bit in the frames bitset to mark the frame as allocated.
 *
 * @param frame_addr The address of the frame to be marked.
 */
static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / 0x1000;          /* Convert frame address to frame index */
    uint32_t idx = INDEX_FROM_BIT(frame);          /* Get the index in the bitset */
    uint32_t off = OFFSET_FROM_BIT(frame);         /* Get the bit offset */
    
    frames[idx] |= (0x1 << off);                   /* Set the corresponding bit to indicate it's used */
}

/**
 * @brief Clears a bit in the frames bitset to mark the frame as free.
 *
 * @param frame_addr The address of the frame to be freed.
 */
static void clear_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / 0x1000;          /* Convert frame address to frame index */
    uint32_t idx = INDEX_FROM_BIT(frame);          /* Get the index in the bitset */
    uint32_t off = OFFSET_FROM_BIT(frame);         /* Get the bit offset */
    
    frames[idx] &= ~(0x1 << off);                  /* Clear the corresponding bit to indicate it's free */
}

/**
 * @brief Tests whether a bit is set in the frames bitset.
 *
 * @param frame_addr The address of the frame to test.
 * @return 1 if the frame is allocated, 0 if it's free.
 */
__attribute__((__unused__))  /* This function is unused but may be useful for debugging */
static uint32_t test_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / 0x1000;          /* Convert frame address to frame index */
    uint32_t idx = INDEX_FROM_BIT(frame);          /* Get the index in the bitset */
    uint32_t off = OFFSET_FROM_BIT(frame);         /* Get the bit offset */
    
    return (frames[idx] & (0x1 << off));           /* Return 1 if the bit is set, 0 if not */
}

/**
 * @brief Finds the first free frame in the bitset.
 *
 * @return The frame index of the first free frame, or -1 if no frames are free.
 */
static uint32_t first_frame(void) {
    uint32_t i, j;

    for (i = 0; i < INDEX_FROM_BIT(nframes); i++) {
        if (frames[i] == 0xFFFFFFFF) {
            /* No free frames in this section, continue to next bitset */
            continue;
        }
        /* At least one bit is free in this section */
        for (j = 0; j < 32; j++) {
            if ((frames[i] & (0x1 << j)) == 0) {
                /* Found the first free frame */
                return (i * 4 * 8 + j);
            }
        }
    }
    return (-1);  /* No free frame found */
}

/**
 * @brief Allocates a frame for the specified page.
 *
 * @param p The page to allocate a frame for.
 * @param is_kernel Flag to specify if the page is for the kernel (1) or user (0).
 * @param is_writeable Flag to specify if the page should be writeable (1) or read-only (0).
 */
void alloc_frame(struct vm_page *p, int is_kernel, int is_writeable) {
    uint32_t idx;

    if (p->p_frame != 0) return; /* Frame already allocated, return immediately */

    idx = first_frame();  /* Get the first free frame index */
    if (idx == -1)
        panic("No free frame.");

    set_frame(idx * 0x1000);  /* Mark this frame as allocated */
    p->p_present = 1;         /* Mark the page as present */
    p->p_frame = idx;
    p->p_rw = (is_writeable) ? 1 : 0; /* Set read/write flag */
    p->p_user = (is_kernel) ? 0 : 1; /* Set user-mode/kernel-mode flag */
}

/**
 * @brief Frees the frame associated with the specified page.
 *
 * @param p The page to free the associated frame for.
 */
void free_frame(struct vm_page *p) {
    if (p->p_frame == 0)
        return; /* No frame to deallocate */

    clear_frame(p->p_frame);  /* Free the frame */
    p->p_frame = 0;           /* Reset the frame address */
}

/**
 * @brief Initializes the paging system, setting up the kernel page directory, heap, and memory mapping.
 */
void init_paging(void*) {
    int i;
    uint32_t mem_end_page;
    extern struct vm_heap *kernel_heap;

    /* Size of physical memory (16MB for now) */
    mem_end_page = 0x10000000;

    nframes = mem_end_page / 0x1000;
    frames = kmalloc0(INDEX_FROM_BIT(nframes));

    /* Create the kernel page directory */
    kernel_directory = kmalloc0_a(sizeof(struct vm_page_directory));

    /* Map some pages in the kernel heap area */
    for (i = VM_KERN_HEAP_START; i < VM_KERN_HEAP_START + VM_KERN_HEAP_INITIAL_SIZE; i += 0x1000)
        get_page(i, 1, kernel_directory);

    /* Allocate the kernel heap before identity mapping */
    struct vm_heap *heap = kmalloc0(sizeof(struct vm_heap));

    /* Identity map physical memory from 0x0 to the end of used memory */
    i = 0;
    while (i < placement_address) {
        alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
        i += 0x1000;
    }

    /* Allocate the pages we mapped earlier */
    for (i = VM_KERN_HEAP_START; i < VM_KERN_HEAP_START + VM_KERN_HEAP_INITIAL_SIZE; i += 0x1000)
        alloc_frame(get_page(i, 0, kernel_directory), 0, 0);

    /* Register the page fault handler */
    register_interrupt_handler(14, page_fault_handler);

    /* Enable paging */
    switch_page_directory(kernel_directory);

    /* Set up the kernel heap for memory allocation */
    kernel_heap = init_heap(heap, VM_KERN_HEAP_START, VM_KERN_HEAP_START +
                            VM_KERN_HEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);

}


void enable_paging() {
    uint32_t cr0;

    // Read the current value of CR0
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));

    // Set the PG bit (31st bit) to enable paging
    cr0 |= (1 << 31);

    // Write the modified value back to CR0
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

void disable_paging() {
    uint32_t cr0;

    // Read the current value of CR0
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));

    // Clear the PG bit (31st bit) to disable paging
    cr0 &= ~(1 << 31);

    // Write the modified value back to CR0
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

/**
 * @brief Switches to the specified page directory for paging operations.
 *
 * @param dir The page directory to switch to.
 */
void switch_page_directory(struct vm_page_directory *dir) {
    uint32_t cr0;

    // Disable paging before switching the page directory
    disable_paging();

    // Update the current directory and set CR3 to the new page directory
    current_directory = dir;
    __asm__ volatile("mov %0, %%cr3" : : "r"(&dir->pd_tblphys));

    // Enable paging after switching the page directory
    enable_paging();
}


/**
 * @brief Retrieves a page from the specified page directory, optionally creating it if it doesn't exist.
 *
 * @param address The virtual address of the page to retrieve.
 * @param create Flag indicating whether to create the page if it doesn't exist.
 * @param dir The page directory to search in.
 * @return A pointer to the requested page, or NULL if it couldn't be found or created.
 */
struct vm_page *get_page(uint32_t address, int create, struct vm_page_directory *dir) {
    address /= 0x1000;
    uint32_t table_idx = address / 1024;

    /* If page table already exists */
    if (dir->pd_tables[table_idx] != NULL)
        return (&dir->pd_tables[table_idx]->pt_pages[address % 1024]);
    else if (create) {
        uint32_t tmp;
        dir->pd_tables[table_idx] = kmalloc0_ap(sizeof(struct vm_page_table), &tmp);
        dir->pd_tblphys[table_idx] = tmp | 0x7;  /* PRESENT, RW, US */
        return (&dir->pd_tables[table_idx]->pt_pages[address % 1024]);
    } else {
        return (0);
    }
}

/**
 * @brief Handles page faults by printing detailed information and halting execution.
 *
 * @param regs The register state at the time of the page fault.
 */
void page_fault_handler(registers_t *regs) {
    uint32_t faulting_address;
    int present, rw, us, reserved, id;

    /* Get the faulting address from CR2 register */
    __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));

    /* Get error code details */
    present = !(regs->err_code & 0x1);  /* Page not present */
    rw = regs->err_code & 0x2;          /* Write operation? */
    us = regs->err_code & 0x4;          /* User-mode? */
    reserved = regs->err_code & 0x8;    /* Reserved bits? */
    id = regs->err_code & 0x10;         /* Instruction fetch? */

    /* Output page fault information */
    printk("Page fault (");
    if (present) printk("present ");
    if (rw) printk("read-only ");
    if (us) printk("user-mode ");
    if (reserved) printk("reserved ");
    printk(") at %x\n", faulting_address);
    panic("Page fault");
}
