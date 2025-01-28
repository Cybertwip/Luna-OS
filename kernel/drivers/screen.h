#ifndef SCREEN_H
#define SCREEN_H

#include "sys/system.h"

/**
 * putch
 * Outputs a single character to the screen at the current cursor position.
 *
 * @param c The character to be displayed.
 */
void putch(char c);

/**
 * cls
 * Clears the entire screen by filling it with blank characters.
 * Resets the cursor position to the top-left corner.
 */
void cls(void);

/**
 * screen_write
 * Outputs a null-terminated string to the screen.
 *
 * @param c Pointer to the null-terminated string to be displayed.
 */
void screen_write(char *c);

#endif /* SCREEN_H */
