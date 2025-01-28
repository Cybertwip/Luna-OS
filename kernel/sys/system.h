#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include <stddef.h>

/* Macro to calculate the number of elements in an array */
#define array_size(x) (sizeof((x)) / sizeof((x)[0]))

/* Macro to trigger a kernel panic with detailed information */
#define panic(s, ...) \
    _panic("%s:%u in %s: " s, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define kassert(msg, cond) (                                       \
		(cond) ?                                           \
		(void)0 :                                          \
		panic("assertion failed: %s (%s)", (msg), (#cond)) \
)

/**
 * outb
 * Writes a byte to the specified I/O port.
 *
 * @param port The I/O port to write to.
 * @param value The byte value to write.
 */
void outb(uint16_t port, uint8_t value);

/**
 * inb
 * Reads a byte from the specified I/O port.
 *
 * @param port The I/O port to read from.
 * @return The byte read from the port.
 */
uint8_t inb(uint16_t port);

/**
 * inw
 * Reads a word (2 bytes) from the specified I/O port.
 *
 * @param port The I/O port to read from.
 * @return The word read from the port.
 */
uint16_t inw(uint16_t port);

/**
 * _panic
 * Triggers a kernel panic, printing a formatted error message and halting the system.
 *
 * @param fmt The format string for the panic message.
 * @param ... Variable arguments to format.
 */
void _panic(const char *fmt, ...);

/**
 * memset
 * Fills a block of memory with a specified value.
 *
 * @param b Pointer to the start of the memory block.
 * @param c Value to set (converted to unsigned char).
 * @param len Number of bytes to set.
 * @return A pointer to the memory block.
 */
void *memset(void *b, int c, size_t len);

/**
 * strlen
 * Calculates the length of a null-terminated string.
 *
 * @param src Pointer to the string.
 * @return The length of the string (excluding the null terminator).
 */
int strlen(char *src);

/**
 * printk
 * Outputs a formatted string to the system log or screen, similar to printf.
 *
 * @param fmt The format string.
 * @param ... Variable arguments to format.
 */
void printk(const char *fmt, ...);

/**
 * bzero
 * Set a block of memory to zero
 *
 * @b: Pointer to the memory block to be set to zero.
 * @len: The number of bytes to set to zero.
 *
 * This function does not return a value.
 */
void bzero(void *b, size_t len);

#endif /* SYSTEM_H */
