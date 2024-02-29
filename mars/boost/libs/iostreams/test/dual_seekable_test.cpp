// (C) Copyright 2014 Jorge Lodos
// (C) Copyright 2008 CodeRage, LLC (turkanis at coderage dot com)
// (C) Copyright 2004-2007 Jonathan Turkanis
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation. 

#include <sstream>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "detail/temp_file.hpp"
#include "detail/verification.hpp"

using namespace std;
using namespace mars_boost;
using namespace mars_boost::iostreams;
using namespace mars_boost::iostreams::test;
using mars_boost::unit_test::test_suite;

void verification_function_seekable_test()
{
    {
        temp_file f;
        fstream io(f.name().c_str(), BOOST_IOS::in | BOOST_IOS::out | BOOST_IOS::binary | BOOST_IOS::trunc);
        BOOST_CHECK_MESSAGE(
            test_seekable_in_chars(io),
            "failed using test_seekable_in_chars"
        );
        io.close();
    }

    {
        temp_file f;
        fstream io(f.name().c_str(), BOOST_IOS::in | BOOST_IOS::out | BOOST_IOS::binary | BOOST_IOS::trunc);
        BOOST_CHECK_MESSAGE(
            test_seekable_in_chunks(io),
            "failed using test_seekable_in_chunks"
        );
        io.close();
    }

    {
        temp_file f;
        fstream io(f.name().c_str(), BOOST_IOS::in | BOOST_IOS::out | BOOST_IOS::binary | BOOST_IOS::trunc);
        for (int i = 0; i < data_reps; ++i)
            io.write(narrow_data(), chunk_size);
        io.seekg(0, BOOST_IOS::beg);
        BOOST_CHECK_MESSAGE(
            test_input_seekable(io),
            "failed using test_input_seekable"
        );
        io.close();
    }

    {
        temp_file f;
        fstream io(f.name().c_str(), BOOST_IOS::in | BOOST_IOS::out | BOOST_IOS::binary | BOOST_IOS::trunc);
        BOOST_CHECK_MESSAGE(
            test_output_seekable(io),
            "failed using test_output_seekable"
        );
        io.close();
    }
}

void verification_function_dual_seekable_test()
{
   {
       stringstream ss(BOOST_IOS::in | BOOST_IOS::out);
       BOOST_CHECK_MESSAGE(
            test_seekable_in_chars(ss),
            "failed using test_seekable_in_chars"
       );
   }

   {
       stringstream ss(BOOST_IOS::in | BOOST_IOS::out);
       BOOST_CHECK_MESSAGE(
            test_seekable_in_chunks(ss),
            "failed using test_seekable_in_chunks"
       );
   }

   {
       string s;
       for (int i = 0; i < data_reps; ++i)
            s.append(narrow_data(), chunk_size);
       stringstream ss(s, BOOST_IOS::in | BOOST_IOS::out);
       BOOST_CHECK_MESSAGE(
            test_input_seekable(ss),
            "failed using test_input_seekable"
       );
   }

   {
       stringstream ss(BOOST_IOS::in | BOOST_IOS::out);
       BOOST_CHECK_MESSAGE(
            test_output_seekable(ss),
            "failed using test_output_seekable"
       );
   }

   {
       stringstream ss(BOOST_IOS::in | BOOST_IOS::out);
       BOOST_CHECK_MESSAGE(
            test_dual_seekable(ss),
            "failed using test_dual_seekable"
       );
   }
}

void dual_seekable_test()
{
   {
       stringstream ss(BOOST_IOS::in | BOOST_IOS::out);
       filtering_stream<dual_seekable> io(ss);
       io.exceptions(BOOST_IOS::failbit | BOOST_IOS::badbit);
       BOOST_CHECK_MESSAGE(
            test_seekable_in_chars(io),
            "failed seeking within a string, in chars"
       );
   }

   {
       stringstream ss(BOOST_IOS::in | BOOST_IOS::out);
       filtering_stream<dual_seekable> io(ss);
       io.exceptions(BOOST_IOS::failbit | BOOST_IOS::badbit);
       BOOST_CHECK_MESSAGE(
            test_seekable_in_chunks(io),
            "failed seeking within a string, in chunks"
       );
   }

   {
       string s;
       for (int i = 0; i < data_reps; ++i)
            s.append(narrow_data(), chunk_size);
       stringstream ss(s, BOOST_IOS::in | BOOST_IOS::out);
       filtering_stream<dual_seekable> io(ss);
       io.exceptions(BOOST_IOS::failbit | BOOST_IOS::badbit);
       BOOST_CHECK_MESSAGE(
            test_input_seekable(io),
            "failed seeking within a string source"
       );
   }

   {
       stringstream ss(BOOST_IOS::in | BOOST_IOS::out);
       filtering_stream<dual_seekable> io(ss);
       io.exceptions(BOOST_IOS::failbit | BOOST_IOS::badbit);
       BOOST_CHECK_MESSAGE(
            test_output_seekable(io),
            "failed seeking within a string sink"
       );
   }

   {
       stringstream ss(BOOST_IOS::in | BOOST_IOS::out);
       filtering_stream<dual_seekable> io(ss);
       io.exceptions(BOOST_IOS::failbit | BOOST_IOS::badbit);
       BOOST_CHECK_MESSAGE(
            test_dual_seekable(io),
            "failed dual seeking within a string"
       );
   }
}

test_suite* init_unit_test_suite(int, char* []) 
{
   test_suite* test = BOOST_TEST_SUITE("dual seekable test");
   test->add(BOOST_TEST_CASE(&verification_function_seekable_test));
   test->add(BOOST_TEST_CASE(&verification_function_dual_seekable_test));
   test->add(BOOST_TEST_CASE(&dual_seekable_test));
   return test;
}
