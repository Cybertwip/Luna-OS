# Specify the target system name (Generic for bare-metal development)
set(CMAKE_SYSTEM_NAME Generic)

# Specify the target processor architecture
set(CMAKE_SYSTEM_PROCESSOR x86)

# Set the cross-compilers
set(CMAKE_C_COMPILER /usr/local/bin/i686-elf-gcc)
set(CMAKE_CXX_COMPILER /usr/local/bin/i686-elf-g++)

# Ensure CMake knows the compilers work
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Disable compiler checks (since we're cross-compiling for bare-metal)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Specify the root path for the target system (if applicable)
# This is optional and depends on your setup
# set(CMAKE_FIND_ROOT_PATH /path/to/target/sysroot)

# Adjust the behavior of find_* commands for cross-compilation
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)    # Don't search for programs in the target sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)     # Only search for libraries in the target sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)     # Only search for headers in the target sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)     # Only search for packages in the target sysroot