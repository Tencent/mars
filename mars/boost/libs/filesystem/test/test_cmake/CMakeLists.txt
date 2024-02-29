# Copyright 2020 Andrey Semashev
#
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt
#
# NOTE: This does NOT run the unit tests for Boost.Filesystem.
#       It only tests, if the CMakeLists.txt file in it's root works as expected

cmake_minimum_required(VERSION 3.5)

project(BoostFilesystemCMakeSelfTest)

# Use experimental superproject to pull library dependencies recursively
set(BOOST_ENABLE_CMAKE 1)
add_subdirectory(../../../.. "${CMAKE_CURRENT_BINARY_DIR}/boost_superproject")

add_definitions(-DBOOST_ALL_NO_LIB)

add_executable(boost_filesystem_cmake_self_test main.cpp)
target_link_libraries(boost_filesystem_cmake_self_test Boost::filesystem)
