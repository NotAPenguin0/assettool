cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(assettool C CXX)

# Allow setting options created with option() through set()
cmake_policy(SET CMP0077 NEW)

set(BUILD_SHARED_LIBS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_executable(assettool "")
target_sources(assettool PRIVATE "src/main.cpp" "src/texture_convert.cpp" "src/mesh_convert.cpp")
target_include_directories(assettool PRIVATE "include/")

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Clang)|(GNU)")
    if (WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -EHa")
    endif()
#    target_compile_options(assettool PUBLIC -fcxx-exceptions)
endif()

add_subdirectory("external")