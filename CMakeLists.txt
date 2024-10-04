cmake_minimum_required(VERSION 3.28)

project(array_list)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_library(array_list_impl INTERFACE)
include_directories(array_list_impl INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(tests)
