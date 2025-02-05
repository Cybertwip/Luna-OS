cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
project(BleskOS)
enable_language(ASM)
enable_language(ASM_NASM)
enable_language(C)
enable_language(CXX)

# Toolchain setup
set(CMAKE_C_COMPILER /usr/local/bin/i686-elf-gcc)
set(CMAKE_CXX_COMPILER /usr/local/bin/i686-elf-g++)
set(CMAKE_ASM_NASM_COMPILER /usr/local/bin/nasm)
set(CMAKE_LINKER /usr/local/bin/i686-elf-ld)

# Compilation flags
set(COMMON_C_FLAGS -m32 -ffreestanding -fno-PIC -fno-stack-protector -Wno-int-conversion -Wno-incompatible-pointer-types)
set(CXX_FLAGS -m32 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-threadsafe-statics)
set(NASM_FLAGS -felf32)

if(WALL)
    set(C_OPTIONS ${COMMON_C_FLAGS} -O0 -Wall -Wno-pointer-sign -Wno-unused-variable)
    set(CXX_OPTIONS ${CXX_FLAGS} -O0 -Wall -Wno-pointer-sign -Wno-unused-variable)
elseif(FAST)
    set(C_OPTIONS ${COMMON_C_FLAGS} -O0)
    set(CXX_OPTIONS ${CXX_FLAGS} -O0)
else()
    set(C_OPTIONS ${COMMON_C_FLAGS} -Os)
    set(CXX_OPTIONS ${CXX_FLAGS} -Os)
endif()

set(KERNEL_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/kernel)
set(OS_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/os)

# Get all source files
file(GLOB_RECURSE KERNEL_C_SOURCES
    ${KERNEL_SRC_DIR}/*.c
)

file(GLOB_RECURSE KERNEL_CPP_SOURCES
    ${KERNEL_SRC_DIR}/*.cpp
)

file(GLOB_RECURSE OS_SOURCES
    ${OS_SRC_DIR}/*.cpp
)

set(USE_THREADS OFF)

set(LUNA_INCLUDES 
${CMAKE_CURRENT_LIST_DIR}/external/EASTL/include
${CMAKE_SOURCE_DIR}/kernel/luna/libc
${CMAKE_SOURCE_DIR}/kernel
${CMAKE_SOURCE_DIR}/os
${CMAKE_SOURCE_DIR}/os/luna/c++
${CMAKE_SOURCE_DIR}/kernel/lib
${CMAKE_CURRENT_LIST_DIR}/external/micro-gl/include
${CMAKE_CURRENT_LIST_DIR}/external/lvgl
${CMAKE_CURRENT_LIST_DIR}/external/gl/include
${CMAKE_CURRENT_LIST_DIR}/external/pdclib/include
${CMAKE_BINARY_DIR}/external/pdclib/configured_include
${CMAKE_BINARY_DIR}/include
${CMAKE_CURRENT_LIST_DIR}/source/
)

# Set include directories 
include_directories(${LUNA_INCLUDES})

set(CONFIG_LV_USE_DEMO_MUSIC ON)
set(CONFIG_LV_USE_DEMO_WIDGETS ON)


# Add subdirectories for dependencies
add_subdirectory(external/EASTL)
add_subdirectory(external/pdclib)
add_subdirectory(external/lvgl)


target_include_directories(EASTL PUBLIC ${LUNA_INCLUDES})

# Configure pdclib for bare metal
target_compile_definitions(pdclibs PUBLIC -DBAREMETAL)

# Configure EASTL for freestanding environment
target_compile_definitions(EASTL PUBLIC 
    -DEA_NO_HAVE_DINKUMWARE_CPP_LIBRARY
    -DEA_NO_HAVE_LIBSTDCPP_LIBRARY
    -DEA_NO_HAVE_LIBCPP_LIBRARY
    -DEA_NO_HAVE_WCHAR_H
    -DEA_WCHAR_T_NON_NATIVE
    -DEA_NO_HAVE_nullptr_t_IMPL
    -DEA_NO_HAVE_PTHREAD_H
    -DEASTL_RTTI_ENABLED=0
    -DEASTL_EXCEPTIONS_ENABLED=0
    -DEA_PLATFORM_LINUX 
    -DEA_COMPILER_NO_STRUCTURED_BINDING
    -DBAREMETAL
)

target_compile_options(EASTL PRIVATE -ffreestanding -fpermissive)
target_compile_definitions(EASTL PRIVATE -D__cplusplus=201402L)

# Build bootloader
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bootloader.bin
    COMMAND ${CMAKE_ASM_NASM_COMPILER} 
        -f bin 
        ${CMAKE_SOURCE_DIR}/source/bootloader/bootloader.asm 
        -o ${CMAKE_BINARY_DIR}/bootloader.bin
    DEPENDS ${CMAKE_SOURCE_DIR}/source/bootloader/bootloader.asm
)

add_library(c_objects ${KERNEL_C_SOURCES})
target_compile_options(c_objects PRIVATE ${C_OPTIONS})
target_include_directories(c_objects PUBLIC ${LUNA_INCLUDES})

target_link_libraries(pdclibs PUBLIC c_objects)
target_link_libraries(lvgl PUBLIC pdclibs)

add_library(cpp_objects ${OS_SOURCES} ${CMAKE_SOURCE_DIR}/source/bleskos.c ${CMAKE_SOURCE_DIR}/source/boot.c )
target_compile_options(cpp_objects PRIVATE ${CXX_OPTIONS} -fpermissive)
target_include_directories(cpp_objects PUBLIC ${LUNA_INCLUDES})
target_link_libraries(cpp_objects PUBLIC EASTL)

# ASM object library
add_library(asm_objects ${CMAKE_SOURCE_DIR}/source/assembly/bleskos.asm)
set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/source/assembly/bleskos.asm 
    PROPERTIES 
    COMPILE_FLAGS "${NASM_FLAGS}"
)


target_link_libraries(asm_objects PUBLIC cpp_objects)

# Link kernel into a binary
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bleskos.bin
    COMMAND ${CMAKE_LINKER}
        -m elf_i386
        -T ${CMAKE_SOURCE_DIR}/source/linker.ld
        --oformat=binary
        -o ${CMAKE_BINARY_DIR}/bleskos.bin
        $<TARGET_FILE:asm_objects>
        $<TARGET_FILE:cpp_objects>
        $<TARGET_FILE:lvgl>
        $<TARGET_FILE:pdclibs>
        $<TARGET_FILE:c_objects>
        $<TARGET_FILE:EASTL>
    DEPENDS asm_objects c_objects cpp_objects pdclibs EASTL lvgl
)

# Create disk image
add_custom_target(image ALL
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/compile
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bleskos.bin ${CMAKE_BINARY_DIR}/bleskos.img
    COMMAND dd if=/dev/zero of=${CMAKE_BINARY_DIR}/bleskos.img bs=1024 count=1440
    COMMAND dd if=${CMAKE_BINARY_DIR}/bootloader.bin of=${CMAKE_BINARY_DIR}/bleskos.img conv=notrunc seek=0
    COMMAND dd if=${CMAKE_BINARY_DIR}/bleskos.bin of=${CMAKE_BINARY_DIR}/bleskos.img conv=notrunc seek=10
    DEPENDS ${CMAKE_BINARY_DIR}/bleskos.bin ${CMAKE_BINARY_DIR}/bootloader.bin
)

# QEMU run target
add_custom_target(run
    COMMAND qemu-system-i386 
    -accel tcg,thread=multi
    -smp 12
    -m 512M
    -display cocoa 
    -cpu qemu32 
    -fda ${CMAKE_BINARY_DIR}/bleskos.img
    -hda ${CMAKE_CURRENT_LIST_DIR}/disk.img
    -boot order=a
    -netdev user,id=net0 
    -device e1000,netdev=net0 
    -object filter-dump,id=f1,netdev=net0,file=/tmp/dump.dat
    DEPENDS image
)

# Custom command to generate the SDK
add_custom_target(sdk
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/sdk/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/sdk/lib

    # Copy headers from external libraries
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/external/EASTL/include ${CMAKE_BINARY_DIR}/sdk/include/c++
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/external/micro-gl/include ${CMAKE_BINARY_DIR}/sdk/include/micro-gl
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/external/lvgl ${CMAKE_BINARY_DIR}/sdk/include/lvgl
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/external/gl/include ${CMAKE_BINARY_DIR}/sdk/include/gl
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/external/pdclib/include ${CMAKE_BINARY_DIR}/sdk/include/libc
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/external/pdclib/configured_include ${CMAKE_BINARY_DIR}/sdk/include/libc/configured_include

    # Exclude kernel and OS headers (do not copy them)

    # Copy precompiled libraries (if any) and rename them
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/external/pdclib/libpdclibs.a ${CMAKE_BINARY_DIR}/sdk/lib/libc.a
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/external/lvgl/lib/liblvgl.a ${CMAKE_BINARY_DIR}/sdk/lib/liblvgl.a
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/external/EASTL/libEASTL.a ${CMAKE_BINARY_DIR}/sdk/lib/libc++.a

    # Copy SDK CMakeLists
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/sdk/CMakeLists.txt ${CMAKE_BINARY_DIR}/sdk/

    DEPENDS ${CMAKE_BINARY_DIR}/bleskos.bin ${CMAKE_BINARY_DIR}/bootloader.bin
)