#ifndef PAGING_H
#define PAGING_H

#include "sys/system.h"
#include "arch/i386/descriptor_tables.h"

#define PAGE_SIZE 0x1000
#define FRAME_SIZE 0x1000
#define PAGE_TABLE_ENTRIES 1024

/* Structure representing a single virtual memory page. */
struct vm_page {
    uint32_t p_present  : 1;   /* Page is present in memory (1) or not (0) */
    uint32_t p_rw       : 1;   /* Read-Write permission (1 for read-write, 0 for read-only) */
    uint32_t p_user     : 1;   /* User-mode (1) or Supervisor-mode (0) access */
    uint32_t p_accessed : 1;   /* Has the page been accessed since the last refresh? (1 for yes, 0 for no) */
    uint32_t p_dirty    : 1;   /* Has the page been written to since the last refresh? (1 for yes, 0 for no) */
    uint32_t p_unused   : 7;   /* Unused and reserved bits (not used by the operating system) */
    uint32_t p_frame    : 20;  /* Frame address (shifted right by 12 bits to address page frames) */
};

/* Structure representing a page table, which holds 1024 pages */
struct vm_page_table {
    struct vm_page pt_pages[PAGE_TABLE_ENTRIES]; /* Array of 1024 pages, each represented by struct vm_page */
};

/* Structure representing a page directory, which holds 1024 page tables */
struct vm_page_directory {
    /*
     * Array of pointers to page tables (each containing 1024 pages).
     * The page directory itself contains up to 1024 entries.
     */
    struct vm_page_table *pd_tables[PAGE_TABLE_ENTRIES];
    
    /*
     * Array of physical addresses of page tables.
     * These are used when loading the page directory into the CR3 register.
     */
    uint32_t pd_tblphys[PAGE_TABLE_ENTRIES];

    /*
     * The physical address of pd_tblphys, which is necessary when the
     * kernel heap is allocated at a different location in memory.
     */
    uint32_t pd_tblphys_addr;
};

/* The kernel's page directory */
extern struct vm_page_directory *kernel_directory;
/* The current page directory */
extern struct vm_page_directory *current_directory;

/*
 * Initializes paging by setting up page directories and enabling paging.
 */
void init_paging(void* mb_info);

/*
 * Switches the current page directory to the specified one.
 * Loads the page directory into the CR3 register.
 * 
 * @param pd: Pointer to the page directory to switch to.
 */
void switch_page_directory(struct vm_page_directory *pd);

/*
 * Retrieves a pointer to the page corresponding to a given virtual address.
 * If the page table is not created and create == 1, it will create the page table.
 *
 * @param address: The virtual address to locate the page for.
 * @param create: Flag indicating whether to create a new page table if needed.
 * @param dir: Pointer to the page directory containing the page table.
 * @return: Pointer to the vm_page structure representing the located page.
 */
struct vm_page *get_page(uint32_t address, int create, struct vm_page_directory *dir);

/*
 * Allocates a frame (physical memory) to a page and sets the relevant flags.
 *
 * @param p: Pointer to the vm_page structure representing the page.
 * @param is_kernel: Flag indicating if the page is for kernel space (1 for kernel, 0 for user).
 * @param is_writeable: Flag indicating if the page is writable (1 for writable, 0 for read-only).
 */
void alloc_frame(struct vm_page *p, int is_kernel, int is_writeable);

/*
 * Frees the frame (physical memory) associated with a page.
 *
 * @param p: Pointer to the vm_page structure representing the page.
 */
void free_frame(struct vm_page *p);

/*
 * Handles page faults. This function is called when the CPU encounters
 * a page fault exception (e.g., accessing a non-present or invalid page).
 *
 * @param regs: Register state at the time of the page fault.
 */
void page_fault_handler(registers_t *regs);

uint32_t virt_to_phys(uint32_t virt_addr, struct vm_page_directory *dir);

void enable_paging();
void disable_paging();

#endif /* ndef PAGING_H */
