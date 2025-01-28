#ifndef __CTYPE_H__
#define __CTYPE_H__

// Check if a character is an alphabetic letter (A-Z or a-z)
#define isalpha(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))

// Check if a character is a control character (0x00-0x1F or 0x7F)
#define isctrl(c)  ((c) >= 0x00 && (c) <= 0x1F) || ((c) == 0x7F)

// Check if a character is a lowercase letter (a-z)
#define islower(c) ((c) >= 'a' && (c) <= 'z')

// Check if a character is a printable character (0x20-0x7E)
#define isprint(c) ((c) >= 0x20 && (c) <= 0x7E)

// Check if a character is a whitespace character (space, tab, newline, etc.)
#define isspace(c) ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))

// Check if a character is an uppercase letter (A-Z)
#define isupper(c) ((c) >= 'A' && (c) <= 'Z')

// Check if a character is a graphical character (printable and not a space)
#define isgraph(c) (isprint(c) && !isspace(c))

// Convert a character to uppercase if it is lowercase
#define toupper(c) (islower(c) ? (c) - 32 : (c))

// Convert a character to lowercase if it is uppercase
#define tolower(c) (isupper(c) ? (c) + 32 : (c))

#endif