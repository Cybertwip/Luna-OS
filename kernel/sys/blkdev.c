#include "blkdev.h"
#include "sync.h"
#include "mm/heap.h"
#include "string.h"

static kmutex_t blkdev_mutex;                  // Mutex for thread-safe operations
static blkdev_class_t *blkdev_classes = NULL;  // Linked list of block device classes
static blkdev_instance_t *blkdev_instances = NULL; // Linked list of block device instances

/* Register a block device class */
int register_blkdev_class(unsigned int major, const char *name,
                          int (*read_blocks)(unsigned int minor, uint32_t block, unsigned int nblocks, void *buffer),
                          int (*write_blocks)(unsigned int minor, uint32_t block, unsigned int nblocks, void *buffer)) {
    blkdev_class_t *class;

    /* Validate input parameters */
    if (!name || !read_blocks || !write_blocks) {
        return -1; // Invalid parameters
    }

    /* Allocate memory for the new class */
    class = kmalloc(sizeof(blkdev_class_t));
    if (!class) {
        return -1; // Memory allocation failed
    }

    /* Initialize the class */
    class->major = major;
    class->name = name;
    class->read_blocks = read_blocks;
    class->write_blocks = write_blocks;
    class->next = NULL;

    /* Add the class to the linked list */
    kmutex_lock(&blkdev_mutex);
    class->next = blkdev_classes;
    blkdev_classes = class;
    kmutex_unlock(&blkdev_mutex);

    return 0; // Success
}

/* Register a block device instance */
int register_blkdev_instance(unsigned int major, unsigned int minor, const char *name,
                             uint32_t block_size, uint64_t capacity) {
    blkdev_instance_t *instance;

    /* Validate input parameters */
    if (!name || block_size == 0 || capacity == 0) {

        return -1; // Invalid parameters
    }

    /* Allocate memory for the new instance */
    instance = kmalloc(sizeof(blkdev_instance_t));
    if (!instance) {
        return -1; // Memory allocation failed
    }

    /* Initialize the instance */
    instance->major = major;
    instance->minor = minor;
    instance->name = name;
    instance->block_size = block_size;
    instance->capacity = capacity;
    instance->next = NULL;

    /* Find the corresponding class */
    kmutex_lock(&blkdev_mutex);
    blkdev_class_t *class = blkdev_classes;
    while (class) {
        if (class->major == major) {
            instance->class = class;
            break;
        }
        class = class->next;
    }
    kmutex_unlock(&blkdev_mutex);

    if (!instance->class) {
        kfree(instance);
        return -1; // No matching class found
    }


    /* Add the instance to the linked list */
    kmutex_lock(&blkdev_mutex);
    instance->next = blkdev_instances;
    blkdev_instances = instance;
    kmutex_unlock(&blkdev_mutex);

    return 0; // Success
}