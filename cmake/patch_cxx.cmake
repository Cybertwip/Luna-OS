# Generate __config_site for libc++
set(CONFIG_SITE_CONTENT "
#ifndef _LIBCPP_CONFIG_SITE
#define _LIBCPP_CONFIG_SITE

// Disable threading support
#define _LIBCPP_HAS_NO_THREADS

// Disable exceptions
#define _LIBCPP_NO_EXCEPTIONS

// Disable RTTI
#define _LIBCPP_NO_RTTI

// Disable filesystem support
#define _LIBCPP_HAS_NO_FILESYSTEM

// Disable locale support
#define _LIBCPP_HAS_NO_LOCALIZATION

// Disable wide character support
#define _LIBCPP_HAS_NO_WIDE_CHARACTERS

// Disable C++20 concepts (optional)
#define _LIBCPP_HAS_NO_CONCEPTS

// Disable C++20 coroutines (optional)
#define _LIBCPP_HAS_NO_COROUTINES

// Define hardening mode
#define _LIBCPP_HARDENING_MODE_DEFAULT _LIBCPP_HARDENING_MODE_NONE
#define _LIBCPP_HARDENING_MODE _LIBCPP_HARDENING_MODE_NONE

// Define nodiscard attribute
#define _LIBCPP_NODISCARD_EXT [[nodiscard]]

// Disable bits/alltypes.h inclusion
#define _LIBCPP_HAS_NO_BITS_ALLTYPES_H

#endif // _LIBCPP_CONFIG_SITE
")

# Write the content to __config_site
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1/__config_site" "${CONFIG_SITE_CONTENT}")

# Generate bits/alltypes.h
set(ALLTYPES_CONTENT "
#ifndef _BITS_ALLTYPES_H
#define _BITS_ALLTYPES_H

// Define minimal types required by libc++
typedef unsigned long size_t;
typedef long ssize_t;
typedef long off_t;
typedef unsigned long mode_t;
typedef unsigned long nlink_t;
typedef unsigned long uid_t;
typedef unsigned long gid_t;
typedef long blksize_t;
typedef long blkcnt_t;
typedef long time_t;
typedef long suseconds_t;

// Define FILE type
typedef __FILE FILE;

#endif // _BITS_ALLTYPES_H
")

# Create the bits directory if it doesn't exist
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1/bits")

# Write the content to bits/alltypes.h
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1/bits/alltypes.h" "${ALLTYPES_CONTENT}")

# Generate bits/limits.h to define LLONG_MIN
set(LIMITS_CONTENT "
#ifndef _BITS_LIMITS_H
#define _BITS_LIMITS_H

#include <climits> // Include standard limits header

// Define LLONG_MIN if not already defined
#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX - 1LL)
#endif

// Define LLONG_MAX if not already defined
#ifndef LLONG_MAX
#define LLONG_MAX __LONG_LONG_MAX__
#endif

#endif // _BITS_LIMITS_H
")

# Write the content to bits/limits.h
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1/bits/limits.h" "${LIMITS_CONTENT}")

# Patch the libc limits.h file to include <climits>
set(LIMITS_H_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/llvm-project/libc/src/__support/CPP/limits.h")
file(READ "${LIMITS_H_PATH}" LIMITS_H_CONTENT)

# Check if <climits> is already included
if(NOT LIMITS_H_CONTENT MATCHES "#include <climits>")
    # Add #include <climits> after the last existing #include
    string(REGEX REPLACE "(#include [^\n]+\n)" "\\1#include <climits>\n" LIMITS_H_CONTENT "${LIMITS_H_CONTENT}")

    # Write the modified content back to limits.h
    file(WRITE "${LIMITS_H_PATH}" "${LIMITS_H_CONTENT}")
    message(STATUS "Patched ${LIMITS_H_PATH} to include <climits>")
else()
    message(STATUS "<climits> is already included in ${LIMITS_H_PATH}")
endif()

# Add critical libc++abi compile definitions
add_compile_definitions(_LIBCXXABI_HAS_NO_THREADS)

# Include the generated headers
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1")