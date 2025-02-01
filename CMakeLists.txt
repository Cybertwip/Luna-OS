cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cmake module path (if exists)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(profile_detection)
include(patch_pdclib)

LOAD_PROFILE("i386" "pc") # Generic for now

set(KERNEL_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/kernel)
set(OS_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/os)

# Get all the files needed
file(GLOB_RECURSE KERNEL_SOURCES
    ${KERNEL_SRC_DIR}/*.c
    ${KERNEL_SRC_DIR}/*.cpp
    ${KERNEL_SRC_DIR}/*.s
)

file(GLOB_RECURSE OS_SOURCES
    ${OS_SRC_DIR}/*.c
    ${OS_SRC_DIR}/*.cpp
)

# these are any additional flags which should be added to the base compiles. Things like 32 bit or elf binaries
set(CFLAGS "-m32 -fcheck-new -ffreestanding -fno-builtin")
set(CFLAGS "${CFLAGS} -nostdlib -finline-functions -fno-stack-protector -fomit-frame-pointer")
set(CFLAGS "${CFLAGS} -mno-sse")
# linker flags and Assembler flags too -Wl,--oformat,binary
set(LINKFLAGS "${ISA_LINKER_FLAGS} ${PLATFORM_LINKER_FLAGS}")
set(ASM_FLAGS "${ISA_ASM_FLAGS}")

project(Luna LANGUAGES C CXX ASM_NASM)
enable_language(ASM_NASM)

add_subdirectory(external/EASTL)

SET (BASE_CFLAGS "${CFLAGS} -std=c11 -fpermissive -fstrength-reduce -fno-builtin-time -fno-builtin-puts -fno-builtin-printf -fno-builtin-function")
SET (BASE_CXXFLAGS "${CFLAGS} -fpermissive -fstrength-reduce -fno-builtin-time -fno-builtin-puts -fno-builtin-printf -fno-builtin-function")

set (CMAKE_C_FLAGS "${BASE_CFLAGS}")
set (CMAKE_CXX_FLAGS "${BASE_CXXFLAGS} -std=c++14 -fno-rtti -fno-exceptions -fno-threadsafe-statics")
SET (CMAKE_ASM_NASM_FLAGS            "-Werror -felf")

if(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	SET (LINKFLAGS "-Wl,--script=${PLATFORM_LAYOUT} -Wl,--omagic ${LINKFLAGS}")
endif(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")


if(${USE_GCC_TOOLCHAIN} OR ${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
	SET (LINKFLAGS "-T ${PLATFORM_LAYOUT} -N ${LINKFLAGS}")
endif(${USE_GCC_TOOLCHAIN} OR ${CMAKE_C_COMPILER_ID} STREQUAL "GNU")

set(USE_THREADS OFF CACHE BOOL "Disable threads" FORCE)
set(AS_C11 OFF CACHE BOOL "Configure with the latest C11" FORCE)
set(BAREMETAL ON CACHE BOOL "Configure as baremetal libc" FORCE)

# Add the subdirectories
add_subdirectory(external/pdclib)
#add_subdirectory(external/leif)

# Finally, Tell CMake to build our kernel.
add_executable(${PROJECT_NAME}.bin ${KERNEL_SOURCES} ${OS_SOURCES})
set_source_files_properties(kernel/arch/i386/interrupt.s PROPERTIES LANGUAGE ASM_NASM)
set_source_files_properties(kernel/arch/i386/thread_asm.s PROPERTIES LANGUAGE ASM_NASM)

target_compile_options(EASTL PRIVATE -ffreestanding -fpermissive)
target_compile_definitions(EASTL PRIVATE 
-DEA_NO_HAVE_DINKUMWARE_CPP_LIBRARY
-DEA_NO_HAVE_LIBSTDCPP_LIBRARY
-DEA_NO_HAVE_LIBCPP_LIBRARY
-DEA_NO_HAVE_WCHAR_H
-DEA_WCHAR_T_NON_NATIVE
-DEA_NO_HAVE_nullptr_t_IMPL
-DEA_NO_HAVE_PTHREAD_H
-DEASTL_SIZE_T_32BIT=0
-DEASTL_RTTI_ENABLED=0
-DEASTL_EXCEPTIONS_ENABLED=0
-DEA_PLATFORM_LINUX 
-DBAREMETAL
-D__cplusplus=201402L 
)

target_include_directories(pdclibs PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/external/EASTL/include  # Ensure libc++ headers are first
    ${CMAKE_SOURCE_DIR}/kernel/luna/libc
    ${CMAKE_SOURCE_DIR}/kernel
    ${CMAKE_SOURCE_DIR}/os
    ${CMAKE_SOURCE_DIR}/kernel/lib
    ${CMAKE_SOURCE_DIR}/kernel/luna/c++
    ${CMAKE_CURRENT_LIST_DIR}/external/micro-gl/include
    ${CMAKE_CURRENT_LIST_DIR}/external/gl/include
    ${CMAKE_CURRENT_LIST_DIR}/external/pdclib/include
    ${CMAKE_BINARY_DIR}/external/pdclib/configured_include
)

#target_link_libraries(leif PUBLIC pdclibs)
target_link_libraries(EASTL PUBLIC pdclibs)

target_link_libraries(${PROJECT_NAME}.bin PRIVATE EASTL)


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nodefaultlibs -Wl,-m,elf_i386 ${LINKFLAGS}")

#include(make_iso)
# ========== QEMU Execution Target ========== #
add_custom_target(run
    DEPENDS ${PROJECT_NAME}.bin
    COMMAND qemu-system-i386 
        -accel tcg,thread=multi
        -smp 4
        -kernel $<TARGET_FILE:${PROJECT_NAME}.bin> 
        -vga std 
        -m 256M
        -display cocoa 
        -machine q35 
        -cpu qemu32 
		-device piix3-ide,id=ide
		-drive id=disk,file=${CMAKE_CURRENT_LIST_DIR}/disk.img,format=raw,if=none,index=0,media=disk
		-device ide-hd,drive=disk,bus=ide.0
		-serial stdio
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Running 32-bit kernel with QEMU"
)
