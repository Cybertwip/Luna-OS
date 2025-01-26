# Minimum CMake version
cmake_minimum_required(VERSION 3.12)

# Project name
project(luna_os)

# Compiler flags for C and C++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -fpermissive -ffreestanding -march=i686 -fno-stack-protector -fno-pic -fno-pie -fno-exceptions -fno-rtti -std=c++20 -fno-threadsafe-statics")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -fpermissive -ffreestanding -march=i686 -fno-stack-protector -fno-pic -fno-pie")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nodefaultlibs -Wl,-m,elf_i386 -T ${CMAKE_SOURCE_DIR}/linker.ld")

# Add picolibc subdirectory
add_subdirectory(external/picolibc)

# Set the picolibc sysroot path
set(PICOLIBC_SYSROOT ${CMAKE_BINARY_DIR}/external/picolibc/picolibc)

# Print the picolibc sysroot path
message(STATUS "Picolibc sysroot: ${PICOLIBC_SYSROOT}")

# Disable LLVM unwinder for libc++abi
set(LIBCXXABI_USE_LLVM_UNWINDER OFF)

# Generate __config_site and bits/alltypes.h on the fly
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_cxx.cmake)

# ========== Critical Additions ========== #
# Force-disable threading in libc++/libc++abi
set(LIBCXX_HAS_NO_THREADS ON CACHE BOOL "Disable all threading" FORCE)
set(LIBCXXABI_HAS_NO_THREADS ON CACHE BOOL "Disable threading in libc++abi" FORCE)

# Explicitly define macros for libc++abi
add_compile_definitions(
    _LIBCPP_HAS_NO_THREADS  # For libc++
    _LIBCXXABI_HAS_NO_THREADS  # For libc++abi (MOST CRITICAL)
)

# Configure libc++abi's CMake flags
set(LIBCXXABI_ENABLE_THREADS OFF CACHE BOOL "Disable threads" FORCE)
set(LIBCXXABI_USE_LLVM_UNWINDER OFF CACHE BOOL "Disable unwinder" FORCE)

# Configure libc++ and libc++abi for barebones environment
set(LIBCXX_ENABLE_THREADS OFF CACHE BOOL "Disable threading support")
set(LIBCXX_ENABLE_FILESYSTEM OFF CACHE BOOL "Disable filesystem support")
set(LIBCXX_ENABLE_RTTI OFF CACHE BOOL "Disable RTTI")
set(LIBCXX_ENABLE_EXCEPTIONS OFF CACHE BOOL "Disable exceptions")
set(LIBCXX_ENABLE_SHARED OFF CACHE BOOL "Disable shared libraries")
set(LIBCXX_ENABLE_STATIC ON CACHE BOOL "Enable static libraries")
set(LIBCXX_HAS_MUSL_LIBC ON CACHE BOOL "Use musl libc")
set(LIBCXX_HAS_EXTERNAL_THREAD_API OFF CACHE BOOL "Use external thread API")

# Force libc++ and libc++abi to build as static libraries
set(LIBCXX_ENABLE_SHARED OFF CACHE BOOL "Disable shared libraries")
set(LIBCXXABI_ENABLE_SHARED OFF CACHE BOOL "Disable shared libraries")

# Disable threading support in libc++ and libc++abi
set(LIBCXX_HAS_THREAD_API OFF CACHE BOOL "Disable threading support")
set(LIBCXXABI_HAS_THREAD_API OFF CACHE BOOL "Disable threading support")

# Add libc++ and libc++abi subdirectories
add_subdirectory(external/llvm-project/libcxx)
add_subdirectory(external/llvm-project/libcxxabi)

# Add the executable target
add_executable(luna_os 
    src/Boot.cpp 
    src/Kernel.cpp 
    src/Engine.cpp 
    src/luna/GPU.cpp 
    src/std/memory.cpp 
    src/std/new.cpp 
    src/std/cxx_support.cpp
    src/std/spinlock.cpp 
)

# Include directories
target_include_directories(luna_os PRIVATE 
    ${PICOLIBC_SYSROOT}/include # Add the picolibc headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/c++/v1 # Add the libc++ headers from the build directory first
    ${CMAKE_CURRENT_BINARY_DIR}/include/bits # Add the custom bits directory
    ${CMAKE_CURRENT_LIST_DIR}/external/llvm-project/libcxx/include
    ${CMAKE_CURRENT_LIST_DIR}/external/llvm-project/libcxxabi/include
    ${CMAKE_CURRENT_BINARY_DIR} # Add the build directory for __config_site
    src
    src/luna
    ${CMAKE_CURRENT_LIST_DIR}/external/micro-gl/include
)

# Link libraries
target_link_libraries(luna_os PRIVATE 
    c
    cxx_static
    cxxabi_static
)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add compiler flags for Release mode
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

# Custom target for running with QEMU
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
    COMMENT "Running the 32-bit kernel with QEMU on macOS"
)