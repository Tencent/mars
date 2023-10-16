cmake_minimum_required (VERSION 2.8)
project (decode_log_file)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lz")

# for zstd
option(ZSTD_BUILD_STATIC "BUILD STATIC LIBRARIES" ON)
option(ZSTD_BUILD_SHARED "BUILD SHARED LIBRARIES" OFF)
set(ZSTD_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../zstd")
set(LIBRARY_DIR ${ZSTD_SOURCE_DIR}/lib)
include(GNUInstallDirs)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../../zstd/build/cmake/lib zstd)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../)


add_executable(decode_log_file decode_log_file.c micro-ecc-master/uECC.c)
target_link_libraries(decode_log_file libzstd_static)