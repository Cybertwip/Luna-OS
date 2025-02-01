#include "strutil.h"
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
