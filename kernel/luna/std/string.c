#include "luna/std/string.h"
#include <ctype.h> // For tolower()
#include <stdlib.h>

// Case-insensitive string comparison
int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int diff = tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        if (diff != 0) {
            return diff;
        }
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

char *strdup(const char *s) {
    if (s == NULL) return NULL;
    
    size_t len = strlen(s) + 1; // +1 for null terminator
    char *new_s = (char*)malloc(len);

    if (new_s == NULL) return NULL;
    
    memcpy(new_s, s, len); // Copy string content + null terminator
    return new_s;
}

#include <stddef.h>

char *strtok_r(char *s, const char *delim, char **saveptr)
{
    char *token;

    if (s == NULL)
        s = *saveptr;

    /* Scan leading delimiters.  */
    s += strspn(s, delim);
    if (*s == '\0')
    {
        *saveptr = s;
        return NULL;
    }

    /* Find end of token.  */
    token = s;
    s = strpbrk(token, delim);
    if (s == NULL)
    {
        /* This token finishes the string.  */
        *saveptr = strchr(token, '\0');
    }
    else
    {
        /* Terminate the token and make *SAVE_PTR point past it.  */
        *s = '\0';
        *saveptr = s + 1;
    }

    return token;
}

/* Helper function to find the first occurrence in the initial segment of S which consists entirely of characters NOT from REJECT. */
size_t strspn(const char *s, const char *reject)
{
    const char *scan;
    for (scan = s; *scan != '\0'; scan++)
    {
        if (strchr(reject, *scan) == NULL)
            break;
    }
    return (scan - s);
}

/* Helper function to search in string S any of the bytes in ACCEPT. */
char *strpbrk(const char *s, const char *accept)
{
    while (*s != '\0')
    {
        const char *a = accept;
        while (*a != '\0')
        {
            if (*a++ == *s)
                return (char *)s;
        }
        ++s;
    }
    return NULL;
}

/* Helper function to locate the first occurrence of C in the string pointed to by S. */
char *strchr(const char *s, int c)
{
    while (*s != '\0')
    {
        if (*s == c)
            return (char *)s;
        s++;
    }
    if (c == '\0')
        return (char *)s;
    return NULL;
}