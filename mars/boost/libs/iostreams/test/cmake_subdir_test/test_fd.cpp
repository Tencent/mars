// Copyright 2021 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/core/lightweight_test.hpp>

namespace io = mars_boost::iostreams;

int main()
{
    io::file_descriptor_source fs( "test.txt" );

    std::string s;
    io::copy( fs, io::back_inserter( s ) );

    BOOST_TEST( s == "=== reference output ===" );

    return mars_boost::report_errors();
}
