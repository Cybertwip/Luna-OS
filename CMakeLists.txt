# Ensure CMake knows the compilers work
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Minimum CMake version
cmake_minimum_required(VERSION 3.12)

# Set the system name to Generic (for bare-metal development)
set(CMAKE_SYSTEM_NAME Generic)

# Project name
project(luna_os)

# Set cross-compilers for 32-bit x86
set(CMAKE_C_COMPILER /usr/local/bin/i686-elf-gcc)
set(CMAKE_CXX_COMPILER /usr/local/bin/i686-elf-g++)

# Compiler flags for C and C++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -Wno-volatile -fpermissive -ffreestanding -march=i686 -fno-stack-protector -fno-pic -fno-pie -fno-exceptions -fno-rtti -std=c++20 -fno-threadsafe-statics")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -Wno-volatile -fpermissive -ffreestanding -march=i686 -fno-stack-protector -fno-pic -fno-pie")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nodefaultlibs -Wl,-m,elf_i386 -T ${CMAKE_SOURCE_DIR}/linker.ld")

# Add the executable target
add_executable(luna_os 
    src/Boot.cpp 
    src/Kernel.cpp 
    src/Engine.cpp 

    src/luna/GPU.cpp 

    src/std/memcpy.cpp 
    src/std/memory.cpp 
    src/std/new.cpp 
    src/std/cxx_support.cpp
    src/std/spinlock.cpp 
)

# Include directories
target_include_directories(luna_os PRIVATE 
    src
    src/luna
    ${CMAKE_CURRENT_LIST_DIR}/external/micro-gl/include
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