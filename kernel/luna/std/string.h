#pragma once

#include <stddef.h> // For size_t

// Standard string functions
int strcasecmp(const char *s1, const char *s2);
char *strdup(const char *s);

char *strtok_r(char *s, const char *delim, char **saveptr);

#include_next <string.h>