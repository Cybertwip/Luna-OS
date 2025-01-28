#ifndef SORTED_ARRAY_H
#define SORTED_ARRAY_H

#include "sys/system.h"

/*
 * A comparison function type definition.
 * This function should return:
 * - a value greater than 0 if the first argument is greater than the second,
 * - 0 if the arguments are equal, and
 * - a value less than 0 if the first argument is less than the second.
 */
typedef int (*cmp_func_t)(void *, void *);

/*
 * Structure for representing a sorted array.
 * It holds:
 * - an array of pointers to elements (sa_array),
 * - the current size of the array (sa_size),
 * - the maximum size of the array (sa_maxsize),
 * - a comparison function (sa_cmp) for comparing elements.
 */
struct sorted_array {
    void    **sa_array;     // Array of pointers to elements.
    size_t  sa_size;        // Current number of elements in the array.
    size_t  sa_maxsize;     // Maximum capacity of the array.
    cmp_func_t sa_cmp;      // Comparison function for sorting.
};

/*
 * Default comparison function.
 * Compares two elements of type void* and returns an integer indicating
 * their relative order.
 */
int default_cmp_func(void *a, void *b);

/*
 * Create a new sorted array.
 * Initializes a sorted array with the specified maximum size and comparison function.
 *
 * Parameters:
 * - maxsize: Maximum number of elements the array can hold.
 * - cmp: The comparison function to be used for sorting.
 *
 * Returns:
 * - A new sorted_array struct.
 */
struct sorted_array new_sorted_array(size_t maxsize, cmp_func_t cmp);

/*
 * Place an existing array into a sorted array structure.
 * This function will initialize the sorted_array structure with an existing
 * array and its comparison function.
 *
 * Parameters:
 * - addr: Pointer to the existing array.
 * - maxsize: Maximum number of elements the array can hold.
 * - cmp: The comparison function to be used for sorting.
 *
 * Returns:
 * - A sorted_array struct initialized with the provided values.
 */
struct sorted_array place_sorted_array(void *addr, size_t maxsize, cmp_func_t cmp);

/**
 * Destroy a sorted array.
 * Frees the memory used by the sorted array and its elements.
 *
 * Parameters:
 * - a: A pointer to the sorted_array struct to be deleted.
 */
void delete_sorted_array(struct sorted_array *a);

/**
 * Add an item into the sorted array.
 * Inserts the given element into the sorted array in the correct position
 * based on the comparison function.
 *
 * Parameters:
 * - a: A pointer to the sorted_array struct.
 * - el: The element to be inserted (of type void*).
 */
void insert_sorted_array(struct sorted_array *a, void *el);

/**
 * Lookup the item at index i in the sorted array.
 * Retrieves the element stored at the given index.
 *
 * Parameters:
 * - a: A pointer to the sorted_array struct.
 * - i: The index of the element to look up.
 *
 * Returns:
 * - A pointer to the element at the specified index.
 */
void *lookup_sorted_array(struct sorted_array *a, uint32_t i);

/**
 * Remove the item at location i from the sorted array.
 * Deletes the element at the specified index and shifts the remaining
 * elements to fill the gap.
 *
 * Parameters:
 * - a: A pointer to the sorted_array struct.
 * - i: The index of the element to be removed.
 */
void remove_sorted_array(struct sorted_array *a, uint32_t i);

#endif /* ndef SORTED_ARRAY_H */
