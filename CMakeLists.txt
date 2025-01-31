cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0079 NEW)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding")

# Ensure Clang/GCC do not use system headers
if(CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()


# Force external build
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)
	message(FATAL_ERROR "You can not use CMake to build from the root of it's source tree! Remove the CMakeCache.txt file from this directory, then create a separate directory (either below this directory or elsewhere), and then re-run CMake from there.")
endif(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)

# Cmake module path (if exists)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(profile_detection)
include(patch_pdclib)

# http://wiki.osdev.org/CMake_Build_System
# This was very helpful in this build system
LOAD_PROFILE("i386" "pc") # Generic for now

# Get all the files needed
file(GLOB KERN_LIB_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kernel/lib/*.c")
file(GLOB KERN_MM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kernel/mm/*.c")
file(GLOB KERN_DRIVERS_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kernel/drivers/*.c")
file(GLOB KERN_SYS_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kernel/sys/*.c")
file(GLOB KERN_MAIN_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kernel/entry/*.cpp")
file(GLOB KERN_LUNA_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kernel/luna/*.cpp")
file(GLOB KERN_LUNA_C_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kernel/luna/*.c")
file(GLOB KERN_LUNA_C_STD_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kernel/luna/std/*.c")
file(GLOB KERN_LUNA_CXX_STD_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kernel/luna/std/*.cpp")


set(KERN_SOURCES ${KERN_TTY_SOURCES} ${KERN_LIB_SOURCES} ${KERN_MM_SOURCES} ${KERN_SYS_SOURCES} ${KERN_DRIVERS_SOURCES} ${KERN_LUNA_C_SOURCES})
# file(GLOB KERN_TTY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "kern/tty/*.c")
file(GLOB LIBC_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "libc/stdlib/*.c")

# Various options we need
option(USE_GCC_TOOLCHAIN "Use the GCC toolchain from tools/mkcrosscompiler_gcc.sh" OFF)

# these are any additional flags which should be added to the base compiles. Things like 32 bit or elf binaries
set(CFLAGS "${ISA_C_FLAGS} -m32 -fcheck-new -ffreestanding -fno-builtin")
set(CFLAGS "${CFLAGS} -nostdlib -finline-functions -fno-stack-protector -fomit-frame-pointer")
set(CFLAGS "${CFLAGS} -mno-sse")
# linker flags and Assembler flags too -Wl,--oformat,binary
set(LINKFLAGS "${ISA_LINKER_FLAGS} ${PLATFORM_LINKER_FLAGS}")
set(ASM_FLAGS "${ISA_ASM_FLAGS}")

# Find clang (if the system has it)
find_program(CLANG clang)
find_program(CLANGXX clang++)

# prefer clang instead of gcc (or whatever shit compiler they're using)
if (CLANG)
	set(CMAKE_C_COMPILER ${CLANG})
endif (CLANG)

# In case we need to compile C++ in the future (which I doubt we will)
if (CLANGXX)
	set(CMAKE_CXX_COMPILER ${CLANGXX})
endif (CLANGXX)


# Finally name our project now that we have picked our compiler.
# this is way down here so the compiler can be tested
project(Luna LANGUAGES C CXX ASM_NASM)
enable_language(ASM_NASM)

# include cmake stuff for checking various system specific defines
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckCCompilerFlag)
include(TestBigEndian)

add_subdirectory(external/EASTL)


# If we're using clang..
if(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	# Clang isn't needy
	SET (BASE_CFLAGS "${CFLAGS} -std=c11 -Werror=implicit-function-declaration")
	
	# Tell clang to pass our linker script to the linker and not try and compile it
	# also tell the linker to set text and data sections to be readable and writable.
	# Do not page align. (see man ld(1))
	SET (LINKFLAGS "-Wl,--script=${PLATFORM_LAYOUT} -Wl,--omagic ${LINKFLAGS}")
	
	SET (CMAKE_C_FLAGS                "-g ${BASE_CFLAGS}")
	SET (CMAKE_C_FLAGS_DEBUG          "-g -D_DEBUG ${BASE_CFLAGS}")
	SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG ${BASE_CFLAGS}")
	SET (CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG ${BASE_CFLAGS}")
	SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g ${BASE_CFLAGS}")

	SET (CMAKE_CXX_FLAGS                "-std=c++14 ${BASE_CFLAGS}")
	SET (CMAKE_CXX_FLAGS_DEBUG          "-g -D_DEBUG ${BASE_CFLAGS}")
	SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG ${BASE_CFLAGS}")
	SET (CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG ${BASE_CFLAGS}")
	SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g ${BASE_CFLAGS}")
	# Error on *ANY* assembler error. We don't want to fuck up assembly.
	SET (CMAKE_ASM_NASM_FLAGS            "-Werror -f elf")
	
endif(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")


# If we're using our custom gcc from /usr/local/cross/bin
if(${USE_GCC_TOOLCHAIN} OR ${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
	
	# additional GCC options cuz GNU.
	
	SET (BASE_CFLAGS "${CFLAGS} -std=c11 -fpermissive -fstrength-reduce -fno-builtin-time -fno-builtin-puts -fno-builtin-printf -fno-builtin-function")
	SET (BASE_CXXFLAGS "${CFLAGS} -fpermissive -fstrength-reduce -fno-builtin-time -fno-builtin-puts -fno-builtin-printf -fno-builtin-function")
	#SET (LINKFLAGS "${LINKFLAGS} -melf_i386")
	
	SET (LINKFLAGS "-T ${PLATFORM_LAYOUT} -N ${LINKFLAGS}")
	
	SET (CMAKE_C_FLAGS                "-g -DHAVE_MMAP=0 -DLACKS_UNISTD_H=1 -DLACKS_SCHED_H=1 -DLACKS_SYS_PARAM_H=1 ${BASE_CFLAGS}") #Fix memory mappings, later
	SET (CMAKE_C_FLAGS_DEBUG          "-g -D_DEBUG ${BASE_CFLAGS}")
	SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG ${BASE_CFLAGS}")
	SET (CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG ${BASE_CFLAGS}")
	SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g ${BASE_CFLAGS}")

	set(CXXFLAGS "${BASE_CXXFLAGS} -std=c++14 -fno-rtti -fno-exceptions -fno-threadsafe-statics")

	SET (CMAKE_CXX_FLAGS                "${CXXFLAGS}")
	SET (CMAKE_CXX_FLAGS_DEBUG          "-g -D_DEBUG ${CXXFLAGS}")
	SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG ${CXXFLAGS}")
	SET (CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG ${CXXFLAGS}")
	SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g ${CXXFLAGS}")
		
	SET (CMAKE_ASM_NASM_FLAGS            "-Werror -felf")

	find_program(GCC_AR ${CMAKE_SYSTEM_PROCESSOR}-elf-ar REQUIRED PATHS /usr/local/cross/bin)
	find_program(GCC_LD ${CMAKE_SYSTEM_PROCESSOR}-elf-ld REQUIRED PATHS /usr/local/cross/bin)
	find_program(GCC_NM ${CMAKE_SYSTEM_PROCESSOR}-elf-nm REQUIRED PATHS /usr/local/cross/bin)
	find_program(GCC_OBJDUMP ${CMAKE_SYSTEM_PROCESSOR}-elf-objdump REQUIRED PATHS /usr/local/cross/bin)
	find_program(GCC_RANLIB ${CMAKE_SYSTEM_PROCESSOR}-elf-ranlib REQUIRED PATHS /usr/local/cross/bin)
	find_program(GCC ${CMAKE_SYSTEM_PROCESSOR}-elf-gcc REQUIRED PATHS /usr/local/cross/bin)
	find_program(GPP ${CMAKE_SYSTEM_PROCESSOR}-elf-g++ REQUIRED PATHS /usr/local/cross/bin)
	message(STATUS "GCC-ar: ${GCC_AR}")
	message(STATUS "GCC-ld: ${GCC_LD}")
	message(STATUS "GCC-nm: ${GCC_NM}")
	message(STATUS "GCC-objdump: ${GCC_OBJDUMP}")
	message(STATUS "GCC-ranlib: ${GCC_RANLIB}")
	message(STATUS "GCC: ${GCC}")
	message(STATUS "G++: ${GPP}")

	SET (CMAKE_AR      "${GCC_AR}")
	SET (CMAKE_LINKER  "${GCC_LD}")
	SET (CMAKE_NM      "${GCC_NM}")
	SET (CMAKE_OBJDUMP "${GCC_OBJDUMP}")
	SET (CMAKE_RANLIB  "${GCC_RANLIB}")
	SET (CMAKE_CXX_COMPILER "${GPP}")
	SET (CMAKE_C_COMPILER "${GCC}")
	
endif(${USE_GCC_TOOLCHAIN} OR ${CMAKE_C_COMPILER_ID} STREQUAL "GNU")

# Concatenate and sort
set(SOURCE_FILES ${LIBC_SOURCES} ${KERN_SOURCES} ${ISA_SRCS} ${PLATFORM_SRCS})
list(SORT SOURCE_FILES)

# Find perl - for run-cc.pl to colorize errors/warnings during compile
find_program(PERL perl)
if(PERL AND NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
	# C++ compiles
	set(CMAKE_CXX_COMPILE_OBJECT "${PERL} ${CMAKE_SOURCE_DIR}/run-cc.pl -q ${CMAKE_CXX_COMPILE_OBJECT}")
	set(CMAKE_CXX_LINK_EXECUTABLE "${PERL} ${CMAKE_SOURCE_DIR}/run-cc.pl -q ${CMAKE_CXX_LINK_EXECUTABLE}")
	set(CMAKE_CXX_CREATE_SHARED_MODULE "${PERL} ${CMAKE_SOURCE_DIR}/run-cc.pl -q ${CMAKE_CXX_CREATE_SHARED_MODULE}")
	# C compiles
	set(CMAKE_C_COMPILE_OBJECT "${PERL} ${CMAKE_SOURCE_DIR}/run-cc.pl -q ${CMAKE_C_COMPILE_OBJECT}")
	set(CMAKE_C_LINK_EXECUTABLE "${PERL} ${CMAKE_SOURCE_DIR}/run-cc.pl -q ${CMAKE_C_LINK_EXECUTABLE}")
	set(CMAKE_C_CREATE_SHARED_MODULE "${PERL} ${CMAKE_SOURCE_DIR}/run-cc.pl -q ${CMAKE_C_CREATE_SHARED_MODULE}")
	# Assembly compiles
	set(CMAKE_ASM_COMPILE_OBJECT "${PERL} ${CMAKE_SOURCE_DIR}/run-cc.pl -q <CMAKE_ASM-ATT_COMPILER> ${ASM_FLAGS} -o <OBJECT> <SOURCE>")
else(PERL AND NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
	# We're not doing colored builds or we're clang.
	# Assembly compiles
	SET(CMAKE_ASM-ATT_COMPILE_OBJECT  "<CMAKE_ASM-ATT_COMPILER> ${ASM_FLAGS} -o <OBJECT> <SOURCE>")
endif(PERL AND NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")


set(USE_THREADS OFF CACHE BOOL "Disable threads" FORCE)
set(AS_C11 OFF CACHE BOOL "Configure with the latest C11" FORCE)
set(BAREMETAL ON CACHE BOOL "Configure as baremetal libc" FORCE)


# Add the subdirectories
add_subdirectory(external/pdclib)

# Finally, Tell CMake to build our kernel.
add_executable(${PROJECT_NAME}.bin ${SOURCE_FILES} ${KERN_MAIN_SOURCES} ${KERN_LUNA_SOURCES} ${KERN_LUNA_C_STD_SOURCES} ${KERN_LUNA_CXX_STD_SOURCES})
set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE C COMPILE_FLAGS "")
set_source_files_properties(kernel/arch/i386/boot.s PROPERTIES LANGUAGE ASM_NASM)
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

target_compile_definitions(${PROJECT_NAME}.bin PRIVATE 
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
)


target_link_libraries(EASTL PUBLIC pdclibs)

target_link_libraries(${PROJECT_NAME}.bin PRIVATE EASTL)

target_include_directories(pdclibs PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/external/EASTL/include  # Ensure libc++ headers are first
    ${CMAKE_SOURCE_DIR}/libc/stdlib
    ${CMAKE_SOURCE_DIR}/kernel
    ${CMAKE_SOURCE_DIR}/kernel/sys
    ${CMAKE_SOURCE_DIR}/kernel/lib
    ${CMAKE_SOURCE_DIR}/kernel/luna
    ${CMAKE_SOURCE_DIR}/kernel/luna/std
    ${CMAKE_CURRENT_LIST_DIR}/external/micro-gl/include
    ${CMAKE_CURRENT_LIST_DIR}/external/pdclib/include
    ${CMAKE_BINARY_DIR}/external/pdclib/configured_include
)

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

add_custom_target(run_vbox
    DEPENDS ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_COMMAND} -E echo "Creating 1024 MB disk image..."
    # Create a raw disk image
    COMMAND dd if=/dev/zero of=${CMAKE_BINARY_DIR}/luna.img bs=512 count=2880
    # Attach the disk image and format it as FAT32
    COMMAND python3 ${CMAKE_SOURCE_DIR}/format_disk.py ${CMAKE_BINARY_DIR}/luna.img
    # Mount the disk image and copy the kernel
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/mnt
    COMMAND hdiutil attach -mountpoint ${CMAKE_BINARY_DIR}/mnt ${CMAKE_BINARY_DIR}/luna.img
    COMMAND cp $<TARGET_FILE:${PROJECT_NAME}.bin> ${CMAKE_BINARY_DIR}/mnt/kernel.elf
    COMMAND hdiutil detach ${CMAKE_BINARY_DIR}/mnt
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/mnt
    # VirtualBox setup
    # COMMAND VBoxManage convertfromraw ${CMAKE_BINARY_DIR}/disk.img ${CMAKE_BINARY_DIR}/disk.vdi --format VDI
    # COMMAND VBoxManage createvm --name "LunaOS" --register
    # COMMAND VBoxManage modifyvm "LunaOS" --memory 1024 --acpi on --boot1 disk --ostype Other
    # COMMAND VBoxManage storagectl "LunaOS" --name "IDE Controller" --add ide
    # COMMAND VBoxManage storageattach "LunaOS" --storagectl "IDE Controller" --port 0 --device 0 --type hdd --medium ${CMAKE_BINARY_DIR}/disk.vdi
    # COMMAND VBoxManage startvm "LunaOS"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running 32-bit kernel with VirtualBox on macOS (1024 MB disk image)"
)


