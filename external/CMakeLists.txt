include(FetchContent)

add_subdirectory("assetlib")

add_library(stb-image STATIC "")
target_sources(stb-image PRIVATE "stb/stb_image.c")

FetchContent_Declare(
	mipgen
	GIT_REPOSITORY https://github.com/NotAPenguin0/mipgen
)

FetchContent_Declare(
	plib 
	GIT_REPOSITORY https://github.com/NotAPenguin0/plib
)

FetchContent_Declare(
	assimp
	GIT_REPOSITORY https://github.com/assimp/assimp
)


set(ASSIMP_BUILD_ZLIB OFF)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)

FetchContent_MakeAvailable(mipgen)
FetchContent_MakeAvailable(plib)
FetchContent_MakeAvailable(assimp)

target_compile_definitions(assimp PRIVATE -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_options(assimp PRIVATE "-w")

target_link_libraries(assettool PRIVATE assetlib stb-image mipgen assimp)
target_include_directories(assettool PRIVATE "assetlib/include" "stb" "${mipgen_SOURCE_DIR}/include" "${plib_SOURCE_DIR}/include")