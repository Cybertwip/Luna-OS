#ifndef __STRING_H__
#define __STRING_H__
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

extern size_t strlen(const char*);
extern char *strchr(const char *s, int c);
extern int strcmp(const char *s1, const char *s2);
extern int strcasecmp(const char *s1, const char *s2);
extern char *strcat(char *dest, const char *src);
extern char *index(const char *s, int c);
extern char *rindex(const char *s, int c);
extern char *strstr(const char *haystack, const char *needle);
extern char *strncpy(char *dest, const char *src, size_t n);
extern char *strcpy(char *dest, const char *src);
extern char *strrev(char *_str);
extern char *strnrev(char *_str, size_t len);

// mem* funcs
extern int memcmp(const void *cmp1, const void *cmp2, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern void *memrev(void *dest, const void *src, size_t n);

// Explicit mem* funcs (used for security)
extern void bzero(void * const buf, const size_t n);
extern void explicit_bzero(void * const buf, const size_t n);
extern int timingsafe_bcmp(const void * const b1, const void * const b2, const size_t n);

// extended libc funcs
extern char *strtoupper(char *dest, const char *src);
extern char *strtolower(char *dest, const char *src);

// Posix extended funcs
extern char *strerror(int errnum);

#ifdef __cplusplus
}
#endif

#endif