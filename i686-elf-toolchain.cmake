# Specify the target system name (Generic for bare-metal development)
set(CMAKE_SYSTEM_NAME Generic)

# Specify the target processor architecture
set(CMAKE_SYSTEM_PROCESSOR i686)

# Set the cross-compilers
set(CMAKE_C_COMPILER /usr/local/bin/i686-elf-gcc)
set(CMAKE_CXX_COMPILER /usr/local/bin/i686-elf-g++)

# Ensure CMake knows the compilers work
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

