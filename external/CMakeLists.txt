add_subdirectory("assetlib")

add_library(stb-image STATIC "")
target_sources(stb-image PRIVATE "stb/stb_image.c")

target_link_libraries(assettool PRIVATE assetlib stb-image)
target_include_directories(assettool PRIVATE "assetlib/include" "stb")