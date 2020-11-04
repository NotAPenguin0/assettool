cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(assettool C CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(assettool "")
target_sources(assettool PRIVATE "src/main.cpp" "src/texture_convert.cpp")
target_include_directories(assettool PRIVATE "include/")

add_subdirectory("external")