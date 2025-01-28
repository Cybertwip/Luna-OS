#include "screen.h"
#include "sys/system.h"

/* Default attribute byte: background (black) | foreground (white) */
#define MON_DEFAULT_ATTR_BYTE  ((0 /* black */ << 4) | (15 /* white */ & 0x0F))
/* Blank character with default attributes */
#define MON_BLANK_CHAR         (0x20 /* space */ | (MON_DEFAULT_ATTR_BYTE << 8))

/* Screen dimensions */
#define MON_WIDTH  80
#define MON_HEIGHT 25

/* Cursor control */
static uint16_t *textmemptr = (uint16_t *)0xB8000; /* Start of video memory */
static uint8_t cursor_x = 0;  /* Current cursor x-coordinate */
static uint8_t cursor_y = 0;  /* Current cursor y-coordinate */

/* Function prototypes */
static void move_csr(void);
static void scroll(void);

/**
 * move_csr
 * Moves the hardware cursor to the current position in the `cursor_x` and `cursor_y` variables.
 */
static void move_csr(void)
{
    uint16_t cursor_location = cursor_y * MON_WIDTH + cursor_x;

    outb(0x3D4, 14);            /* Set high cursor byte. */
    outb(0x3D5, cursor_location >> 8); /* Send high byte. */
    
    outb(0x3D4, 15);            /* Set low cursor byte. */
    outb(0x3D5, cursor_location & 0xFF); /* Send low byte. */
}

/**
 * scroll
 * Scrolls the screen upwards by one line if the cursor moves beyond the bottom of the screen.
 */
static void scroll(void)
{
    int i;

    /* Check if scrolling is needed */
    if (cursor_y >= MON_HEIGHT) {
        /* Copy each line to the line above */
        for (i = 0; i < MON_WIDTH * (MON_HEIGHT - 1); i++) {
            textmemptr[i] = textmemptr[i + MON_WIDTH];
        }

        /* Clear the last line */
        for (i = MON_WIDTH * (MON_HEIGHT - 1); i < MON_WIDTH * MON_HEIGHT; i++) {
            textmemptr[i] = MON_BLANK_CHAR;
        }

        /* Adjust cursor position to the last line */
        cursor_y = MON_HEIGHT - 1;
    }
}

/**
 * putch
 * Outputs a single character to the screen at the current cursor position.
 * Handles special characters like backspace, tab, carriage return, and newline.
 *
 * @param c The character to display.
 */
void putch(char c)
{
    uint8_t bg_color = 0; /* Background color (black) */
    uint8_t fg_color = 2; /* Foreground color (green) */
    uint8_t attr_byte = (bg_color << 4) | (fg_color & 0x0F);
    uint16_t attr = attr_byte << 8;
    uint16_t *location;

    /* Handle special characters */
    if (c == 0x08 && cursor_x > 0) {        /* Backspace */
        cursor_x--;
    } else if (c == 0x09) {                 /* Tab */
        cursor_x = (cursor_x + 8) & ~(8 - 1);
    } else if (c == '\r') {                 /* Carriage return */
        cursor_x = 0;
    } else if (c == '\n') {                 /* Newline */
        cursor_x = 0;
        cursor_y++;
    } else if (c >= ' ') {                  /* Printable characters */
        location = textmemptr + (cursor_x++ + MON_WIDTH * cursor_y);
        *location = c | attr;
    }

    /* Wrap cursor to the next line if necessary */
    if (cursor_x >= MON_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    /* Scroll the screen if needed */
    scroll();

    /* Move the hardware cursor */
    move_csr();
}

/**
 * cls
 * Clears the screen by filling it with blank characters and resets the cursor position.
 */
void cls(void)
{
    int i;

    /* Fill screen with blank characters */
    for (i = 0; i < MON_WIDTH * MON_HEIGHT; i++) {
        textmemptr[i] = MON_BLANK_CHAR;
    }

    /* Reset cursor position */
    cursor_x = cursor_y = 0;
    move_csr();
}

/**
 * screen_write
 * Outputs a null-terminated string to the screen by calling `putch` for each character.
 *
 * @param c Pointer to the null-terminated string to display.
 */
void screen_write(char *c)
{
    int i = 0;

    /* Iterate through each character in the string */
    while (c[i]) {
        putch(c[i++]);
    }
}
