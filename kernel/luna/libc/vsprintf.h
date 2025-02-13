#ifdef __cplusplus
extern "C" {
#endif

#ifndef VSPRINTF_H
#define VSPRINTF_H

#include <stdarg.h>

int vsprintf(char *buf, const char *fmt, va_list args);

#endif

#ifdef __cplusplus
}
#endif
