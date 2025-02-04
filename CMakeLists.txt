cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
project(BleskOS)
enable_language(ASM_NASM)
enable_language(C)
enable_language(CXX)

# Toolchain setup
set(CMAKE_C_COMPILER /usr/local/bin/i686-elf-gcc)
set(CMAKE_CXX_COMPILER /usr/local/bin/i686-elf-g++)
set(CMAKE_ASM_NASM_COMPILER /usr/local/bin/nasm)
set(CMAKE_LINKER /usr/local/bin/i686-elf-ld)

# Compilation flags
# Separate flags for C and C++ to avoid inappropriate use of -std=gnu99 for C++
set(COMMON_C_FLAGS -m32 -ffreestanding -fno-PIC -fno-stack-protector)
set(CXX_FLAGS -m32 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector)
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

# Bootloader (binary format)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bootloader.bin
    COMMAND ${CMAKE_ASM_NASM_COMPILER} 
        -f bin 
        ${CMAKE_SOURCE_DIR}/source/bootloader/bootloader.asm 
        -o ${CMAKE_BINARY_DIR}/bootloader.bin
    DEPENDS ${CMAKE_SOURCE_DIR}/source/bootloader/bootloader.asm
)

# Separate C and C++ libraries for appropriate compilation flags
add_library(c_objects OBJECT ${CMAKE_SOURCE_DIR}/source/bleskos.c)
target_compile_options(c_objects PRIVATE ${C_OPTIONS})

add_library(cpp_objects OBJECT ${CMAKE_SOURCE_DIR}/kernel/arch/i386/boot.cpp)
target_compile_options(cpp_objects PRIVATE ${CXX_OPTIONS} -fpermissive)
target_include_directories(cpp_objects PRIVATE source kernel/luna/libc)

# ASM Object Library
add_library(asm_objects OBJECT ${CMAKE_SOURCE_DIR}/source/assembly/bleskos.asm)
set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/source/assembly/bleskos.asm 
    PROPERTIES 
    COMPILE_FLAGS "${NASM_FLAGS}"
)

# Custom linking command to create flat binary
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bleskos.bin
    COMMAND ${CMAKE_LINKER}
        -m elf_i386
        -T ${CMAKE_SOURCE_DIR}/source/linker.ld
        --oformat=binary
        -o ${CMAKE_BINARY_DIR}/bleskos.bin
        $<TARGET_OBJECTS:asm_objects>
        $<TARGET_OBJECTS:c_objects>
        $<TARGET_OBJECTS:cpp_objects>
    DEPENDS asm_objects c_objects cpp_objects
)

add_custom_target(image ALL
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/compile
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bleskos.bin ${CMAKE_BINARY_DIR}/bleskos.img
    COMMAND dd if=/dev/zero of=${CMAKE_BINARY_DIR}/bleskos.img bs=1024 count=1440
    COMMAND dd if=${CMAKE_BINARY_DIR}/bootloader.bin of=${CMAKE_BINARY_DIR}/bleskos.img conv=notrunc seek=0
    COMMAND dd if=${CMAKE_BINARY_DIR}/bleskos.bin of=${CMAKE_BINARY_DIR}/bleskos.img conv=notrunc seek=10
    DEPENDS ${CMAKE_BINARY_DIR}/bleskos.bin ${CMAKE_BINARY_DIR}/bootloader.bin
)

add_custom_target(run
    COMMAND qemu-system-i386 
    --accel tcg,thread=multi
    -fda ${CMAKE_BINARY_DIR}/bleskos.img
    -hda ${CMAKE_CURRENT_LIST_DIR}/disk.img
    -boot order=a
    -netdev user,id=net0 
    -device e1000,netdev=net0 
    -object filter-dump,id=f1,netdev=net0,file=/tmp/dump.dat
    DEPENDS image
)