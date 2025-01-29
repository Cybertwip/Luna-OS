# Patch CMakeLists.txt to replace ${CMAKE_BINARY_DIR}/configured_include
set(CMAKE_LISTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/pdclib/CMakeLists.txt")
file(READ "${CMAKE_LISTS_PATH}" CMAKE_LISTS_CONTENT)

# Replace all occurrences of ${CMAKE_BINARY_DIR}/configured_include
string(REGEX REPLACE "\\$\\{CMAKE_BINARY_DIR\\}/configured_include" 
    "${CMAKE_BINARY_DIR}/external/pdclib/configured_include" 
    CMAKE_LISTS_CONTENT "${CMAKE_LISTS_CONTENT}")

# Write modified content back
file(WRITE "${CMAKE_LISTS_PATH}" "${CMAKE_LISTS_CONTENT}")
message(STATUS "Replaced ${CMAKE_BINARY_DIR}/configured_include with ${CMAKE_BINARY_DIR}/external/pdclib/configured_include in ${CMAKE_LISTS_PATH}")