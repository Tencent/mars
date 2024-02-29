# Copyright 2018, 2019 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.16)

project(cmake_subdir_test LANGUAGES CXX)

add_subdirectory(../.. boostorg/smart_ptr)
add_subdirectory(../../../assert boostorg/assert)
add_subdirectory(../../../config boostorg/config)
add_subdirectory(../../../core boostorg/core)
add_subdirectory(../../../move boostorg/move)
add_subdirectory(../../../static_assert boostorg/static_assert)
add_subdirectory(../../../throw_exception boostorg/throw_exception)
add_subdirectory(../../../type_traits boostorg/type_traits)

add_executable(quick ../quick.cpp)
target_link_libraries(quick Boost::smart_ptr Boost::core)

enable_testing()
add_test(quick quick)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>)
