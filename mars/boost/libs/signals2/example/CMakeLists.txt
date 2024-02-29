# Copyright 2019 Mike Dev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt
#
# NOTE: CMake support for Boost.Signals2 is currently experimental at best
#       and the interface is likely to change in the future

file( GLOB example_src_files *.cpp )

find_package(Threads REQUIRED)

foreach( file IN LISTS example_src_files )

    get_filename_component( example_name ${file} NAME_WE )

    add_executable( boost_signals2_example_${example_name} ${file} )
    target_link_libraries( boost_signals2_example_${example_name}
        PUBLIC
            Boost::signals2
            Boost::io
            Threads::Threads
    )

endforeach()
