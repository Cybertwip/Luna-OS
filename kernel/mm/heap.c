
#include "paging.h"
#include "heap.h"
#include "mm/kmalloc.h"

/* The kernel's heap */
struct vm_heap *kernel_heap;

/* Extern reference to the kernel's page directory */
extern struct vm_page_directory *kernel_directory;

/**
 * find_smallest_hole - Finds the smallest hole in the heap that can fit a given block of memory.
 * 
 * @size: The size of the memory to be allocated.
 * @page_align: A flag indicating if the memory should be page-aligned.
 * @heap: The heap in which we are searching for space.
 * 
 * Returns the index of the smallest hole that fits, or -1 if no suitable hole is found.
 */
static int32_t find_smallest_hole(size_t size, int page_align, struct vm_heap *heap)
{
    uint32_t i = 0;

    // Iterate through the sorted list of heap blocks
    while (i < heap->h_index.sa_size) {
        struct vm_heap_header *hdr = lookup_sorted_array(&heap->h_index, i);
        
        // Check if the memory should be page-aligned
        if (page_align) {
            uint32_t location = (uint32_t)hdr;
            int32_t offset = 0;

            // Align the starting point to the next page boundary
            if ((location + sizeof(struct vm_heap_header)) & 0xFFFFF000)
                offset = 0x1000 - (location + sizeof(struct vm_heap_header)) % 0x1000;

            int32_t hole_size = (int32_t)hdr->hh_size - offset;
            
            // Check if the hole is large enough
            if (hole_size >= (int32_t)size)
                break;
        }
        else if (hdr->hh_size >= size) {
            // If no page alignment is required, just check size
            break;
        }

        i++;
    }

    // Return -1 if no suitable hole is found
    if (i == heap->h_index.sa_size)
        i = -1;

    return i;
}

/** 
 * vm_heap_header_cmp - Compares two heap headers based on their size.
 * 
 * @a: Pointer to the first heap header.
 * @b: Pointer to the second heap header.
 * 
 * Returns a comparison value for sorting (negative if a < b, positive if a > b).
 */
static int vm_heap_header_cmp(void *a, void *b)
{
    return (((struct vm_heap_header *)a)->hh_size - ((struct vm_heap_header *)b)->hh_size);
}

/** 
 * new_heap - Creates a new heap structure with the given parameters.
 * 
 * @start: The start address of the heap.
 * @end: The end address of the heap.
 * @max: The maximum address the heap can grow to.
 * @su: Whether the heap is for user space.
 * @ro: Whether the heap is read-only.
 * 
 * Returns a pointer to the newly created heap.
 */
struct vm_heap *new_heap(uint32_t start, uint32_t end, uint32_t max, int su, int ro)
{
    struct vm_heap *heap = kmalloc0(sizeof(struct vm_heap));

    // Panic if memory allocation fails
    if (heap == NULL)
        panic("kmalloc");

    return init_heap(heap, start, end, max, su, ro);
}

/**
 * init_heap - Initializes the heap structure with the given parameters.
 * 
 * @heap: The heap to be initialized.
 * @start: The start address of the heap.
 * @end: The end address of the heap.
 * @max: The maximum address the heap can grow to.
 * @su: Whether the heap is for user space.
 * @ro: Whether the heap is read-only.
 * 
 * Returns the initialized heap.
 */
struct vm_heap *init_heap(struct vm_heap *heap, uint32_t start, uint32_t end, uint32_t max, int su, int ro)
{
    // Ensure the start and end addresses are page-aligned
    kassert("start of the heap is page aligned", (start % 0x1000) == 0);
    kassert("end of the heap is page aligned", (end % 0x1000) == 0);

    // Initialize the sorted array index for the heap
    heap->h_index = place_sorted_array((void *)start, VM_HEAP_INDEX_SIZE, &vm_heap_header_cmp);

    // Adjust the start address to accommodate the index
    start += sizeof(void *) * VM_HEAP_INDEX_SIZE;

    // Align the start address to the next page boundary
    if (start & 0xFFFFF000) {
        start &= 0xFFFFF000;
        start += 0x1000;
    }

    // Set the heap's start, end, and max addresses
    heap->h_addr_start = start;
    heap->h_addr_end = end;
    heap->h_addr_max = max;
    heap->h_su = su;
    heap->h_ro = ro;

    // Initialize the first large hole in the heap
    struct vm_heap_header *hole = (void *)start;
    hole->hh_size = end - start;
    hole->hh_magic = VM_HEAP_HDR_MAGIC;
    hole->hh_is_hole = 1;

    // Insert the hole into the sorted array index
    insert_sorted_array(&heap->h_index, hole);

    return heap;
}

/** 
 * expand - Expands the heap to a larger size by allocating additional pages.
 * 
 * @new_size: The new size to expand the heap to.
 * @heap: The heap to expand.
 */
static void expand(size_t new_size, struct vm_heap *heap)
{
    // Sanity check to ensure the new size is greater than the current size
    kassert("expand to a greater size", new_size > (heap->h_addr_end - heap->h_addr_start));

    // Align the new size to the next page boundary
    if (new_size & 0xFFFFF000) {
        new_size &= 0xFFFFF000;
        new_size += 0x1000;
    }

    // Ensure we don't overflow the heap
    kassert("don't overflow the heap", heap->h_addr_start + new_size <= heap->h_addr_max);

    // Get the current size and expand page-by-page
    uint32_t old_size = heap->h_addr_end - heap->h_addr_start;
    uint32_t i = old_size;
    while (i < new_size) {
        alloc_frame(get_page(heap->h_addr_start + i, 1, kernel_directory), heap->h_su, heap->h_ro);
        i += 0x1000;  // Increment by page size
    }

    // Update the heap's end address
    heap->h_addr_end = heap->h_addr_start + new_size;
}

/**
 * contract - Contracts the heap to a smaller size by freeing pages.
 * 
 * @new_size: The new size to contract the heap to.
 * @heap: The heap to contract.
 * 
 * Returns the new size of the heap after contraction.
 */
static uint32_t contract(uint32_t new_size, struct vm_heap *heap)
{
    // Sanity check to ensure the new size is smaller than the current size
    kassert("expand contract a smaller size", new_size < (heap->h_addr_end - heap->h_addr_start));

    // Align the new size to the next page boundary
    if (new_size & 0x1000) {
        new_size &= 0x1000;
        new_size += 0x1000;
    }

    // Ensure the new size doesn't go below the minimum heap size
    if (new_size < VM_HEAP_MIN_SIZE)
        new_size = VM_HEAP_MIN_SIZE;

    // Get the current size and free pages until the heap is the new size
    uint32_t old_size = heap->h_addr_end - heap->h_addr_start;
    uint32_t i = old_size - 0x1000;
    while (new_size < i) {
        free_frame(get_page(heap->h_addr_start + i, 0, kernel_directory));
        i -= 0x1000;  // Decrement by page size
    }

    // Update the heap's end address
    heap->h_addr_end = heap->h_addr_start + new_size;

    return new_size;
}

/**
 * alloc - Allocates memory from the heap.
 * 
 * This function searches for a suitable free block in the heap and returns a pointer
 * to a newly allocated memory block. If no suitable block is found, the heap is expanded.
 *
 * @size:        The requested size for the allocation.
 * @page_align:  If non-zero, ensures the allocation is page-aligned.
 * @heap:        The heap from which memory is allocated.
 * 
 * Returns: A pointer to the allocated memory block.
 */
void *alloc(uint32_t size, int page_align, struct vm_heap *heap)
{
    // Account for the size of header and footer
    size_t new_size = size + sizeof(struct vm_heap_header) + sizeof(struct vm_heap_footer);

    // Search for the smallest hole that fits the requested size
    int i = find_smallest_hole(new_size, page_align, heap);

    if (i == -1) { // No suitable hole found
        // Save current heap details
        uint32_t old_length = heap->h_addr_end - heap->h_addr_start;
        uint32_t old_end_address = heap->h_addr_end;

        // Expand the heap and update the new size
        expand(old_length + new_size, heap);
        uint32_t new_length = heap->h_addr_end - heap->h_addr_start;

        // Find the last header in the heap
        i = 0;
        uint32_t idx = -1; 
        uint32_t value = 0x0;

        // Search for the endmost header
        while (i < heap->h_index.sa_size) {
            uint32_t tmp = (uint32_t)lookup_sorted_array(&heap->h_index, i);
            if (tmp > value) {
                value = tmp;
                idx = i;
            }
            i++;
        }

        // Add a new header if no headers found
        if (idx == -1) {
            struct vm_heap_header *header = (struct vm_heap_header *)old_end_address;
            header->hh_magic = VM_HEAP_HDR_MAGIC;
            header->hh_size = new_length - old_length;
            header->hh_is_hole = 1;

            struct vm_heap_footer *footer = (struct vm_heap_footer *)(old_end_address + header->hh_size - sizeof(struct vm_heap_footer));
            footer->hf_magic = VM_HEAP_FTR_MAGIC;
            footer->hf_header = header;

            insert_sorted_array(&heap->h_index, (void*)header);
        } else {
            // Adjust the last header and footer
            struct vm_heap_header *header = lookup_sorted_array(&heap->h_index, idx);
            header->hh_size += new_length - old_length;

            struct vm_heap_footer *footer = (struct vm_heap_footer *)((uint32_t)header + header->hh_size - sizeof(struct vm_heap_footer));
            footer->hf_magic = VM_HEAP_FTR_MAGIC;
            footer->hf_header = header;
        }

        // Retry the allocation with the updated heap
        return (alloc(size, page_align, heap));
    }

    // We found a suitable hole, let's process it
    struct vm_heap_header *orig_hole_header = lookup_sorted_array(&heap->h_index, i);
    uint32_t orig_hole_pos = (uint32_t)orig_hole_header;
    uint32_t orig_hole_size = orig_hole_header->hh_size;

    // Check if we should split the hole into two parts
    if ((orig_hole_size - new_size) < (sizeof(struct vm_heap_header) + sizeof(struct vm_heap_footer))) {
        size += orig_hole_size - new_size;
        new_size = orig_hole_size;
    }

    // Handle page alignment if requested
    if (page_align && (orig_hole_pos & 0xFFFFF000)) {
        uint32_t new_location = orig_hole_pos + 0x1000 - (orig_hole_pos & 0xFFF) - sizeof(struct vm_heap_header);
        struct vm_heap_header *hole_header = (struct vm_heap_header *)orig_hole_pos;
        hole_header->hh_size = 0x1000 - (orig_hole_pos & 0xFFF) - sizeof(struct vm_heap_header);
        hole_header->hh_magic = VM_HEAP_HDR_MAGIC;
        hole_header->hh_is_hole = 1;

        struct vm_heap_footer *hole_footer = (struct vm_heap_footer *)((uint32_t)new_location - sizeof(struct vm_heap_footer));
        hole_footer->hf_magic = VM_HEAP_FTR_MAGIC;
        hole_footer->hf_header = hole_header;

        orig_hole_pos = new_location;
        orig_hole_size -= hole_header->hh_size;
    } else {
        // Remove the hole from the index if not page-aligned
        remove_sorted_array(&heap->h_index, i);
    }

    // Overwrite the original header with the new block header
    struct vm_heap_header *block_header = (struct vm_heap_header *)orig_hole_pos;
    block_header->hh_magic = VM_HEAP_HDR_MAGIC;
    block_header->hh_is_hole = 0;
    block_header->hh_size = new_size;

    // Set the footer for the allocated block
    struct vm_heap_footer *block_footer = (struct vm_heap_footer *)(orig_hole_pos + sizeof(struct vm_heap_header) + size);
    block_footer->hf_magic = VM_HEAP_FTR_MAGIC;
    block_footer->hf_header = block_header;

    // If there is remaining space, create a new hole
    if (orig_hole_size - new_size > 0) {
        struct vm_heap_header *hole_header = (struct vm_heap_header *)(orig_hole_pos + sizeof(struct vm_heap_header) + size + sizeof(struct vm_heap_footer));
        hole_header->hh_magic = VM_HEAP_HDR_MAGIC;
        hole_header->hh_is_hole = 1;
        hole_header->hh_size = orig_hole_size - new_size;

        struct vm_heap_footer *hole_footer = (struct vm_heap_footer *)((uint32_t)hole_header + orig_hole_size - new_size - sizeof(struct vm_heap_footer));
        if ((uint32_t)hole_footer < heap->h_addr_end) {
            hole_footer->hf_magic = VM_HEAP_FTR_MAGIC;
            hole_footer->hf_header = hole_header;
        }

        // Add the new hole to the index
        insert_sorted_array(&heap->h_index, (void*)hole_header);
    }

    // Return the pointer to the allocated block, skipping the header
    return (void *)((uint32_t)block_header + sizeof(struct vm_heap_header));
}

/**
 * free - Frees a previously allocated block of memory.
 *
 * This function marks the memory block as free and tries to coalesce adjacent free blocks
 * to reduce fragmentation.
 *
 * @p:    The pointer to the memory block to be freed.
 * @heap: The heap from which memory was allocated.
 */
void free(void *p, struct vm_heap *heap)
{
    // If the pointer or heap is null, return immediately
    if (p == NULL || heap == NULL)
        return;

    // Get the header and footer for the block being freed
    struct vm_heap_header *header = (struct vm_heap_header *)((uint32_t)p - sizeof(struct vm_heap_header));
    struct vm_heap_footer *footer = (struct vm_heap_footer *)((uint32_t)header + header->hh_size - sizeof(struct vm_heap_footer));

    // Sanity checks for header and footer magic values
    kassert("header magic match", header->hh_magic == VM_HEAP_HDR_MAGIC);
    kassert("footer magic match", footer->hf_magic == VM_HEAP_FTR_MAGIC);

    // Mark the block as a hole
    header->hh_is_hole = 1;
    char do_add = 1;

    // Attempt to unify with the block on the left
    struct vm_heap_footer *test_footer = (struct vm_heap_footer *)((uint32_t)header - sizeof(struct vm_heap_footer));
    if (test_footer->hf_magic == VM_HEAP_FTR_MAGIC && test_footer->hf_header->hh_is_hole == 1) {
        uint32_t cache_size = header->hh_size;
        header = test_footer->hf_header;
        footer->hf_header = header;
        header->hh_size += cache_size;
        do_add = 0; // No need to add it again since it's already in the index
    }

    // Attempt to unify with the block on the right
    struct vm_heap_header *test_header = (struct vm_heap_header *)((uint32_t)footer + sizeof(struct vm_heap_footer));
    if (test_header->hh_magic == VM_HEAP_HDR_MAGIC && test_header->hh_is_hole) {
        header->hh_size += test_header->hh_size;
        test_footer = (struct vm_heap_footer *)((uint32_t)test_header + test_header->hh_size - sizeof(struct vm_heap_footer));
        footer = test_footer;

        // Find and remove the right block from the index
        uint32_t i = 0;
        while (i < heap->h_index.sa_size && lookup_sorted_array(&heap->h_index, i) != test_header)
            i++;
        kassert("found an item while unifying right", i < heap->h_index.sa_size);
        remove_sorted_array(&heap->h_index, i);
    }

    // If the footer is at the end of the heap, attempt to contract the heap
    if ((uint32_t)footer + sizeof(struct vm_heap_footer) == heap->h_addr_end) {
        uint32_t old_length = heap->h_addr_end - heap->h_addr_start;
        uint32_t new_length = contract((uint32_t)header - heap->h_addr_start, heap);

        // If the block will still exist, adjust its size
        if (header->hh_size - (old_length - new_length) > 0) {
            header->hh_size -= old_length - new_length;
            footer = (struct vm_heap_footer *)((uint32_t)header + header->hh_size - sizeof(struct vm_heap_footer));
            footer->hf_magic = VM_HEAP_FTR_MAGIC;
            footer->hf_header = header;
        } else {
            // Otherwise, remove the block from the index if it no longer exists
            uint32_t i = 0;
            while (i < heap->h_index.sa_size && lookup_sorted_array(&heap->h_index, i) != test_header)
                i++;
            if (i < heap->h_index.sa_size)
                remove_sorted_array(&heap->h_index, i);
        }
    }

    // If necessary, add the block back to the free holes index
    if (do_add)
        insert_sorted_array(&heap->h_index, header);
}