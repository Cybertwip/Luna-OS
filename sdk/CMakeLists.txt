cmake_minimum_required(VERSION 3.15)
project(Mars_SDK)

# Interface libraries for each component
add_library(c INTERFACE)
target_include_directories(c INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/libc)
target_include_directories(c INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/libc/configured_include)

add_library(cxx INTERFACE)
target_include_directories(cxx INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/c++)

add_library(micro-gl INTERFACE)
target_include_directories(micro-gl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/micro-gl)

add_library(lvgl INTERFACE)
target_include_directories(lvgl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/lvgl)

add_library(GL INTERFACE)
target_include_directories(GL INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/GL)

# Aggregate SDK target
add_library(Mars_SDK INTERFACE)
target_include_directories(Mars_SDK INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_directories(Mars_SDK INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
