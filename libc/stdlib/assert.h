// kernel_assert.h
#pragma once

#include "sys/system.h"

#ifdef NDEBUG
  #define assert(condition) ((void)0)
#else
  #define assert(condition) \
    do { \
      if (!(condition)) { \
        panic("Assertion failed: %s, file %s, line %d", \
            #condition, __FILE__, __LINE__); \
      } \
    } while (0)
#endif

// Assume kernel_panic is defined elsewhere (halts the system).