# Copyright 2019 Mike Dev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required( VERSION 3.5...3.20 )
project( boost_any VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX )

add_library( boost_any INTERFACE )
add_library( Boost::any ALIAS boost_any )

target_include_directories( boost_any INTERFACE include )

target_link_libraries( boost_any
    INTERFACE
        Boost::config
        Boost::throw_exception
        Boost::type_index
)
