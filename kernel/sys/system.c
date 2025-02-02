#include "sys/system.h"
#include "drivers/screen.h"
#include "vsprintf.h"

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

/**
 * outb
 * Writes a byte to the specified I/O port.
 *
 * @param port The I/O port to write to.
 * @param value The byte value to write.
 */
void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

/**
 * inb
 * Reads a byte from the specified I/O port.
 *
 * @param port The I/O port to read from.
 * @return The byte read from the port.
 */
uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

/**
 * inw
 * Reads a word (2 bytes) from the specified I/O port.
 *
 * @param port The I/O port to read from.
 * @return The word read from the port.
 */
uint16_t inw(uint16_t port)
{
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}



void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %w1" : : "a"(value), "Nd"(port));
}

unsigned long inl (unsigned short _port){
    __asm__ volatile ("cli");
    unsigned long rv;
    __asm__ __volatile__ ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    __asm__ volatile ("sti");
    return rv;
}

void outl (unsigned short _port, unsigned long _data){
    __asm__ volatile ("cli");
    __asm__ __volatile__ ("outl %1, %0" : : "dN" (_port), "a" (_data));
    __asm__ volatile ("sti");
}


/**
 * memset
 * Fills a block of memory with a specified value.
 *
 * @param b Pointer to the start of the memory block.
 * @param c Value to set (converted to unsigned char).
 * @param len Number of bytes to set.
 * @return A pointer to the memory block.
 */
void *memset(void *b, int c, size_t len)
{
    char *bb = b;

    while (len-- > 0) {
        *bb++ = (char)c;
    }

    return b;
}

/**
 * strlen
 * Calculates the length of a null-terminated string.
 *
 * @param src Pointer to the string.
 * @return The length of the string (excluding the null terminator).
 */
int strlen(char *src)
{
    int len = 0;

    while (*src++) {
        len++;
    }

    return len;
}

/**
 * printk
 * Outputs a formatted string to the screen, similar to printf.
 *
 * @param fmt The format string.
 * @param ... Variable arguments to format.
 */
void printk(const char *fmt, ...)
{
    static char buf[1024]; /* Buffer to hold the formatted string */
    va_list args;
    int len;

    va_start(args, fmt);
    len = vsprintf(buf, fmt, args); /* Format the string */
    va_end(args);

    buf[len] = '\0'; /* Null-terminate the string */
    screen_write(buf);
}

/**
 * _panic
 * Halts the system after printing a kernel panic message.
 *
 * @param fmt The format string for the panic message.
 * @param ... Variable arguments to format.
 */
void _panic(const char *fmt, ...)
{
    static char buf[1024];
    va_list args;

    /* Print the panic header */
    screen_write("\nKernel panic!\n--------------------------\n");

    /* Format the panic message */
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    /* Write the formatted panic message */
    screen_write(buf);

    /* Print the footer */
    screen_write("\n--------------------------\n");

    /* Halt the system indefinitely */
    for (;;);
}

/**
 * bzero - Set a block of memory to zero
 *
 * This function sets the specified memory block to zero, similar to 
 * `memset` but specifically for zeroing out memory. It is generally 
 * used for clearing buffers, structures, or arrays.
 *
 * @b: Pointer to the memory block to be set to zero.
 * @len: The number of bytes to set to zero.
 *
 * This function does not return a value.
 */
void bzero(void *b, size_t len)
{
    memset(b, 0, len);
}
