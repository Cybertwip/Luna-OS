# patch_cxx.cmake

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

# Notify the user
message(STATUS "Generated __config_site for libc++ in ${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1")

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

# Notify the user
message(STATUS "Generated bits/alltypes.h in ${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1/bits")

# Add critical libc++abi compile definitions
add_compile_definitions(_LIBCXXABI_HAS_NO_THREADS)
message(STATUS "Force-disabled threading in libc++abi")