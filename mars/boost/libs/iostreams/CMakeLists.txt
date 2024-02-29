# Copyright 2020, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.16)

project(boost_iostreams VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

add_library(boost_iostreams
  src/file_descriptor.cpp
  src/mapped_file.cpp
)

function(boost_iostreams_option name description package version found target) # sources...

  find_package(${package} ${version} QUIET)

  if(${found} AND TARGET ${target})

    option(${name} ${description} ON)

  else()

    option(${name} ${description} OFF)

  endif()

  if(${name})

    find_package(${package} ${version} REQUIRED)
    target_sources(boost_iostreams PRIVATE ${ARGN})
    target_link_libraries(boost_iostreams PRIVATE ${target})

  endif()

endfunction()

boost_iostreams_option(BOOST_IOSTREAMS_ENABLE_ZLIB "Boost.Iostreams: Enable ZLIB support" ZLIB "" ZLIB_FOUND ZLIB::ZLIB src/zlib.cpp src/gzip.cpp)
boost_iostreams_option(BOOST_IOSTREAMS_ENABLE_BZIP2 "Boost.Iostreams: Enable BZip2 support" BZip2 "" BZIP2_FOUND BZip2::BZip2 src/bzip2.cpp)
boost_iostreams_option(BOOST_IOSTREAMS_ENABLE_LZMA "Boost.Iostreams: Enable LZMA support" LibLZMA "" LIBLZMA_FOUND LibLZMA::LibLZMA src/lzma.cpp)
boost_iostreams_option(BOOST_IOSTREAMS_ENABLE_ZSTD "Boost.Iostreams: Enable Zstd support" zstd "1.0" zstd_FOUND zstd::libzstd_shared src/zstd.cpp)

include(CheckCXXSourceCompiles)

function(iostreams_check var source incs libs defs)

  set(CMAKE_REQUIRED_INCLUDES "${incs}")
  list(APPEND CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/build")
  set(CMAKE_REQUIRED_LIBRARIES "${libs}")
  set(CMAKE_REQUIRED_DEFINITIONS "${defs}")
  check_cxx_source_compiles("#include \"${source}\"" ${var})
  set(${var} ${${var}} PARENT_SCOPE)

endfunction()

set(_lzma_mt "")

if(BOOST_IOSTREAMS_ENABLE_LZMA)

  iostreams_check(BOOST_IOSTREAMS_HAS_LZMA_CPUTHREADS has_lzma_cputhreads.cpp "" "LibLZMA::LibLZMA" "")

  if(BOOST_IOSTREAMS_HAS_LZMA_CPUTHREADS)
    set(_lzma_mt " (multithreaded)")
  else()
    target_compile_definitions(boost_iostreams PRIVATE BOOST_IOSTREAMS_LZMA_NO_MULTITHREADED=1)
  endif()

endif()

message(STATUS "Boost.Iostreams: ZLIB ${BOOST_IOSTREAMS_ENABLE_ZLIB}, BZip2 ${BOOST_IOSTREAMS_ENABLE_BZIP2}, LZMA ${BOOST_IOSTREAMS_ENABLE_LZMA}${_lzma_mt}, Zstd ${BOOST_IOSTREAMS_ENABLE_ZSTD}")

unset(_lzma_mt)

add_library(Boost::iostreams ALIAS boost_iostreams)

target_include_directories(boost_iostreams PUBLIC include)

target_link_libraries(boost_iostreams
  PUBLIC
    Boost::assert
    Boost::config
    Boost::core
    Boost::detail
    Boost::function
    Boost::integer
    Boost::iterator
    Boost::mpl
    Boost::preprocessor
    Boost::random
    Boost::range
    Boost::regex
    Boost::smart_ptr
    Boost::static_assert
    Boost::throw_exception
    Boost::type_traits
    Boost::utility
  PRIVATE
    Boost::numeric_conversion
)

target_compile_definitions(boost_iostreams
  PUBLIC BOOST_IOSTREAMS_NO_LIB
  # Source files already define BOOST_IOSTREAMS_SOURCE
  # PRIVATE BOOST_IOSTREAMS_SOURCE
)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(boost_iostreams PUBLIC BOOST_IOSTREAMS_DYN_LINK)
else()
  target_compile_definitions(boost_iostreams PUBLIC BOOST_IOSTREAMS_STATIC_LINK)
endif()

if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")

  add_subdirectory(test)

endif()
