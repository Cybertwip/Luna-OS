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

# Read the existing content of limits.h
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

# Patch the libc big_int.h file to include <limits>
set(BIG_INT_H_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/llvm-project/libc/src/__support/big_int.h")

# Read the existing content of big_int.h
file(READ "${BIG_INT_H_PATH}" BIG_INT_H_CONTENT)

# Check if <limits> is already included
if(NOT BIG_INT_H_CONTENT MATCHES "#include <limits>")
    # Add #include <limits> near the top of the file
    string(REGEX REPLACE "(#include [^\n]+\n)" "\\1#include <limits>\n" BIG_INT_H_CONTENT "${BIG_INT_H_CONTENT}")

    # Write the modified content back to big_int.h
    file(WRITE "${BIG_INT_H_PATH}" "${BIG_INT_H_CONTENT}")
    message(STATUS "Patched ${BIG_INT_H_PATH} to include <limits>")
else()
    message(STATUS "<limits> is already included in ${BIG_INT_H_PATH}")
endif()

# Patch the numeric_limits.h file to include the specialization for unsigned long long
set(NUMERIC_LIMITS_H_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/llvm-project/libc/include/__support/CPP/numeric_limits.h")

# Read the existing content of numeric_limits.h
file(READ "${NUMERIC_LIMITS_H_PATH}" NUMERIC_LIMITS_H_CONTENT)

# Check if the specialization for unsigned long long is already present
if(NOT NUMERIC_LIMITS_H_CONTENT MATCHES "numeric_limits<unsigned long long>")
    # Add the specialization for unsigned long long
    set(SPECIALIZATION_CONTENT "
namespace __llvm_libc_common_utils::cpp {
    template <>
    class numeric_limits<unsigned long long> {
    public:
        static constexpr unsigned long long min() noexcept {
            return 0;
        }
        static constexpr unsigned long long max() noexcept {
            return ULLONG_MAX;
        }
        static constexpr int digits = sizeof(unsigned long long) * CHAR_BIT;
    };
}
")

    # Append the specialization to the file
    file(APPEND "${NUMERIC_LIMITS_H_PATH}" "${SPECIALIZATION_CONTENT}")
    message(STATUS "Patched ${NUMERIC_LIMITS_H_PATH} to include specialization for unsigned long long")
else()
    message(STATUS "Specialization for unsigned long long is already present in ${NUMERIC_LIMITS_H_PATH}")
endif()

# Patch wchar.h to fix __restrict usage
set(WCHAR_H_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/picolibc/newlib/libc/include/wchar.h")

# Read the existing content of wchar.h
file(READ "${WCHAR_H_PATH}" WCHAR_H_CONTENT)

# Fix the __restrict usage
string(REPLACE 
    "wchar_t *fgetws (wchar_t *__restrict, int, __FILE *__restrict);" 
    "wchar_t *fgetws (wchar_t *__restrict ws, int n, __FILE *__restrict stream);" 
    WCHAR_H_CONTENT "${WCHAR_H_CONTENT}"
)

string(REPLACE 
    "int      fputws (const wchar_t *__restrict, __FILE *__restrict);" 
    "int      fputws (const wchar_t *__restrict ws, __FILE *__restrict stream);" 
    WCHAR_H_CONTENT "${WCHAR_H_CONTENT}"
)

# Write the modified content back to wchar.h
file(WRITE "${WCHAR_H_PATH}" "${WCHAR_H_CONTENT}")
message(STATUS "Patched ${WCHAR_H_PATH} to fix __restrict usage")

# Patch machine/ieeefp.h to guard against redefinition
set(IEEEFP_H_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/picolibc/newlib/libc/include/machine/ieeefp.h")

# Read the existing content of ieeefp.h
file(READ "${IEEEFP_H_PATH}" IEEEFP_H_CONTENT)

# Add a guard to prevent redefinition of __SIZEOF_LONG_DOUBLE__
if(NOT IEEEFP_H_CONTENT MATCHES "#ifndef __SIZEOF_LONG_DOUBLE__")
    set(IEEEFP_H_CONTENT "
#ifndef __SIZEOF_LONG_DOUBLE__
#define __SIZEOF_LONG_DOUBLE__ 16
#endif

${IEEEFP_H_CONTENT}
")
    # Write the modified content back to ieeefp.h
    file(WRITE "${IEEEFP_H_PATH}" "${IEEEFP_H_CONTENT}")
    message(STATUS "Patched ${IEEEFP_H_PATH} to guard __SIZEOF_LONG_DOUBLE__")
else()
    message(STATUS "__SIZEOF_LONG_DOUBLE__ is already guarded in ${IEEEFP_H_PATH}")
endif()

# Add critical libc++abi compile definitions
add_compile_definitions(_LIBCXXABI_HAS_NO_THREADS)