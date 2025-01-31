#pragma once

#include <stddef.h> // For size_t

// Standard string functions
int strcasecmp(const char *s1, const char *s2);
char *strdup(const char *s);

char *strtok_r(char *s, const char *delim, char **saveptr);
size_t strspn(const char *s, const char *reject);
char *strpbrk(const char *s, const char *accept);
char *strchr(const char *s, int c);

#include_next <string.h>