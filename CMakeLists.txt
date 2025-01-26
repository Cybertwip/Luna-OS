# Minimum CMake version
cmake_minimum_required(VERSION 3.12)

# Enable CMP0079 policy to allow linking against targets not built in the same directory
cmake_policy(SET CMP0079 NEW)

# Project name
project(luna_os)

# Standard configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Base flags for both C/C++
# Base flags for both C/C++
set(COMMON_FLAGS "-m32 -ffreestanding -march=i686 -fno-stack-protector -fno-pic -fno-pie -DNDEBUG -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L -D__STDC_WANT_LIB_EXT1__=1 -D_FORTIFY_SOURCE=2")

# Trim any trailing spaces or newlines
string(STRIP "${COMMON_FLAGS}" COMMON_FLAGS)

# Set C flags explicitly
set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=c11 -fpermissive")

# Set C++ flags explicitly
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=c++20 -fpermissive -fno-exceptions -fno-threadsafe-statics")

# ========== Library Configuration ========== #
# Libc++ settings
set(LIBCXX_ENABLE_THREADS OFF CACHE BOOL "Disable threading" FORCE)
set(LIBCXX_ENABLE_FILESYSTEM OFF CACHE BOOL "Disable filesystem" FORCE)
set(LIBCXX_ENABLE_RTTI OFF CACHE BOOL "Disable RTTI" FORCE)
set(LIBCXX_ENABLE_EXCEPTIONS OFF CACHE BOOL "Disable exceptions" FORCE)
set(LIBCXX_USE_COMPILER_RT OFF CACHE BOOL "Disable compiler-rt" FORCE)
set(LIBCXX_ENABLE_STATIC ON CACHE BOOL "Static build" FORCE)
set(LIBCXX_ENABLE_SHARED OFF CACHE BOOL "Disable shared" FORCE)

# Libc++abi settings
set(LIBCXXABI_USE_LLVM_UNWINDER OFF CACHE BOOL "Disable unwinder" FORCE)
set(LIBCXXABI_ENABLE_THREADS OFF CACHE BOOL "Disable threads" FORCE)
set(LIBCXXABI_ENABLE_EXCEPTIONS OFF CACHE BOOL "Disable exceptions" FORCE)
set(LIBCXXABI_SILENT_TERMINATE ON CACHE BOOL "Silent terminate" FORCE)
set(LIBCXXABI_ENABLE_SHARED OFF CACHE BOOL "Static build" FORCE)


# Generate configuration files
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_cxx.cmake)

# ========== Build System Configuration ========== #
# Linker configuration
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} 
    -nostdlib -nodefaultlibs 
    -Wl,-m,elf_i386 -T ${LINKER_SCRIPT}"
)

# Generate picolibc config
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/external/picolibc/picolibc.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/picolibc.h
    @ONLY
)

# ========== Dependencies ========== #
add_subdirectory(external/picolibc)
add_subdirectory(external/llvm-project/libcxx)
add_subdirectory(external/llvm-project/libcxxabi)

set(PICOLIBC_SYSROOT ${CMAKE_BINARY_DIR}/external/picolibc/picolibc)
message(STATUS "Picolibc sysroot: ${PICOLIBC_SYSROOT}")

# ========== Target Configuration ========== #
# Main executable
add_executable(luna_os 
    src/Boot.cpp 
    src/Kernel.cpp 
    src/Engine.cpp 
    src/luna/GPU.cpp 
    src/std/memory.cpp 
    src/std/new.cpp 
    src/std/cxx_support.cpp
    src/std/spinlock.cpp
    src/stubs.cpp  # Critical system stubs
)

# Include directories
target_include_directories(luna_os PRIVATE 
    ${PICOLIBC_SYSROOT}
    ${CMAKE_CURRENT_BINARY_DIR}/include  
    ${CMAKE_CURRENT_LIST_DIR}/external/picolibc/newlib/libc/include
    ${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1
    ${CMAKE_CURRENT_BINARY_DIR}/include/bits
    ${CMAKE_CURRENT_LIST_DIR}/external/llvm-project/libcxx/include
    ${CMAKE_CURRENT_LIST_DIR}/external/llvm-project/libcxxabi/include
    ${CMAKE_CURRENT_BINARY_DIR}
    src
    src/luna
    ${CMAKE_CURRENT_LIST_DIR}/external/micro-gl/include
)

target_include_directories(cxx_static PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1  # Prioritize C++ headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/bits    # Additional C++ headers
    ${PICOLIBC_SYSROOT}                         # PicoC library headers
    ${CMAKE_CURRENT_BINARY_DIR}/include         # Other includes
    ${CMAKE_CURRENT_LIST_DIR}/external/picolibc/newlib/libc/include  # C Standard Library headers
)

target_include_directories(cxxabi_static PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1  # Prioritize C++ headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/bits    # Additional C++ headers
    ${PICOLIBC_SYSROOT}                         # PicoC library headers
    ${CMAKE_CURRENT_BINARY_DIR}/include         # Other includes
    ${CMAKE_CURRENT_LIST_DIR}/external/picolibc/newlib/libc/include  # C Standard Library headers
)

# Link libraries
target_link_libraries(luna_os PRIVATE 
    c
    cxx_static
    cxxabi_static
)

target_link_libraries(cxx_static PRIVATE 
    c
)

target_link_libraries(cxxabi_static PRIVATE 
    c
)


# ========== Build Type Configuration ========== #
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# ========== QEMU Execution Target ========== #
add_custom_target(run
    DEPENDS luna_os
    COMMAND qemu-system-i386 
        -kernel $<TARGET_FILE:luna_os> 
        -vga std 
        -m 4096M 
        -display cocoa 
        -machine q35 
        -cpu qemu32 
        -smp 2 
        -serial stdio
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Running 32-bit kernel with QEMU"
)

# Define __STDC_VERSION__ for C11
target_compile_definitions(cxx_static PRIVATE
    __STDC_VERSION__=201112L
)

# ========== Critical System Configurations ========== #
# Define __STDC_VERSION__ for C11
target_compile_definitions(cxx_static PRIVATE
    __STDC_VERSION__=201112L
    SIZEOF_LONG_DOUBLE=16  # Use non-reserved name
)

# Repeat for cxxabi_static if necessary
target_compile_definitions(cxxabi_static PRIVATE
    __STDC_VERSION__=201112L
    SIZEOF_LONG_DOUBLE=16
)

# Undefine __restrict and use standard restrict
target_compile_options(cxx_static PRIVATE
    -U__restrict
)

# ========== Critical System Configurations ========== #
target_compile_definitions(luna_os PRIVATE
    _LIBCPP_HAS_NO_THREADS
    _LIBCXXABI_HAS_NO_THREADS
    _LIBCPP_HAS_NO_MONOTONIC_CLOCK  # If not implementing chrono support
)
