#include "sorted_array.h"
#include "mm/kmalloc.h"

/**
 * Default comparison function for sorting elements.
 * Compares two pointers by subtracting their values.
 * 
 * @param a Pointer to the first element.
 * @param b Pointer to the second element.
 * 
 * @return An integer representing the result of the subtraction (a - b).
 */
int default_cmp_func(void *a, void *b)
{
    return (a - b);
}

/**
 * Creates a new sorted array with dynamic memory allocation.
 * Initializes the array with a given maximum size and a custom comparison function.
 * 
 * @param maxsize The maximum number of elements the sorted array can hold.
 * @param cmp The comparison function used to sort the elements.
 * 
 * @return A sorted array structure initialized with the given parameters.
 */
struct sorted_array new_sorted_array(size_t maxsize, cmp_func_t cmp)
{
    struct sorted_array a;
    a.sa_array = kmalloc0(maxsize * sizeof(void *));
    if (a.sa_array == NULL)
        panic("kmalloc");
    a.sa_size = 0;
    a.sa_maxsize = maxsize;
    a.sa_cmp = cmp;

    return (a);
}

/**
 * Initializes an existing sorted array at a given memory address.
 * The array is cleared, and its size is set to 0.
 * 
 * @param addr A pointer to the pre-allocated memory for the array.
 * @param maxsize The maximum number of elements the sorted array can hold.
 * @param cmp The comparison function used to sort the elements.
 * 
 * @return A sorted array structure initialized with the given parameters.
 */
struct sorted_array place_sorted_array(void *addr, size_t maxsize, cmp_func_t cmp)
{
    struct sorted_array a;

    a.sa_array = addr;
    bzero(a.sa_array, maxsize * sizeof(void *));
    a.sa_size = 0;
    a.sa_maxsize = maxsize;
    a.sa_cmp = cmp;

    return (a);
}

/**
 * Deletes a sorted array by freeing the allocated memory.
 * 
 * @param a A pointer to the sorted array structure to be deleted.
 */
void delete_sorted_array(struct sorted_array *a)
{
    kfree(a->sa_array);
}

/**
 * Inserts an element into the sorted array, maintaining the sorted order.
 * The element is inserted at the appropriate position based on the comparison function.
 * 
 * @param a A pointer to the sorted array structure.
 * @param el The element to be inserted into the array.
 */
void insert_sorted_array(struct sorted_array *a, void *el)
{
    uint32_t iterator = 0;

    kassert("has a cmp function", a->sa_cmp != NULL);
    kassert("has enough room", a->sa_size < a->sa_maxsize);

    // Find the correct position for the element.
    while (iterator < a->sa_size && a->sa_cmp(a->sa_array[iterator], el) < 0)
        iterator++;

    // Insert the element at the found position.
    if (iterator == a->sa_size) // Just add at the end.
        a->sa_array[a->sa_size++] = el;
    else {
        void *tmp = a->sa_array[iterator];
        a->sa_array[iterator] = el;
        while (iterator < a->sa_size) {
            void *tmp2 = a->sa_array[++iterator];
            a->sa_array[iterator] = tmp;
            tmp = tmp2;
        }
        a->sa_size++;
    }
}

/**
 * Looks up an element at a specific index in the sorted array.
 * 
 * @param a A pointer to the sorted array structure.
 * @param i The index of the element to be looked up.
 * 
 * @return A pointer to the element at the specified index.
 */
void *lookup_sorted_array(struct sorted_array *a, uint32_t i)
{
    kassert("index in range", i < a->sa_size);
    return a->sa_array[i];
}

/**
 * Removes an element at a specific index from the sorted array.
 * The array is restructured to maintain the sorted order.
 * 
 * @param a A pointer to the sorted array structure.
 * @param i The index of the element to be removed.
 */
void remove_sorted_array(struct sorted_array *a, uint32_t i)
{
    kassert("index in range", i < a->sa_size);

    // Shift all elements after the removed one to fill the gap.
    while (i < a->sa_size) {
        a->sa_array[i] = a->sa_array[i + 1];
        i++;
    }
    a->sa_size--;
}
