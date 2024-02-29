// (C) Copyright 2008 CodeRage, LLC (turkanis at coderage dot com)
// (C) Copyright 2004-2007 Jonathan Turkanis
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.

#include <string>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "detail/sequence.hpp"

using namespace std;
using namespace mars_boost;
using namespace mars_boost::iostreams;
using namespace mars_boost::iostreams::test;
using mars_boost::unit_test::test_suite;  
namespace io = mars_boost::iostreams;

template<class T> struct basic_test_alloc: std::allocator<T>
{
    basic_test_alloc()
    {
    }

    basic_test_alloc( basic_test_alloc const& /*other*/ )
    {
    }

    template<class U>
    basic_test_alloc( basic_test_alloc<U> const & /*other*/ )
    {
    }

    template<class U> struct rebind
    {
        typedef basic_test_alloc<U> other;
    };
};

typedef basic_test_alloc<char> bzip2_alloc;

void bzip2_test()
{
    text_sequence data;
    BOOST_CHECK(
        test_filter_pair( bzip2_compressor(), 
                          bzip2_decompressor(), 
                          std::string(data.begin(), data.end()) )
    );
    BOOST_CHECK(
        test_filter_pair( basic_bzip2_compressor<bzip2_alloc>(), 
                          basic_bzip2_decompressor<bzip2_alloc>(), 
                          std::string(data.begin(), data.end()) )
    );
    BOOST_CHECK(
        test_filter_pair( bzip2_compressor(), 
                          bzip2_decompressor(), 
                          std::string() )
    );
    {
        filtering_istream strm;
        strm.push( bzip2_compressor() );
        strm.push( null_source() );
    }
    {
        filtering_istream strm;
        strm.push( bzip2_decompressor() );
        strm.push( null_source() );
    }
}    

void multiple_member_test()
{
    const int num_sequences = 10;
    text_sequence      data;
    std::vector<char>  temp, dest;

    // Write compressed data to temp, several times in succession
    filtering_ostream out;
    out.push(bzip2_compressor());
    for(int i = 0; i < num_sequences; ++i)
    {
        out.push(io::back_inserter(temp));
        io::copy(make_iterator_range(data), out);
    }

    // Read compressed data from temp into dest
    filtering_istream in;
    in.push(bzip2_decompressor());
    in.push(array_source(&temp[0], temp.size()));
    io::copy(in, io::back_inserter(dest));

    // Check that dest consists of as many copies of data as were provided
    BOOST_REQUIRE_EQUAL(data.size() * num_sequences, dest.size());
    for(int i = 0; i < num_sequences; ++i)
        BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin() + i * dest.size() / num_sequences));

    dest.clear();
    io::copy(
        array_source(&temp[0], temp.size()),
        io::compose(bzip2_decompressor(), io::back_inserter(dest)));

    // Check that dest consists of as many copies of data as were provided
    BOOST_REQUIRE_EQUAL(data.size() * num_sequences, dest.size());
    for(int i = 0; i < num_sequences; ++i)
        BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin() + i * dest.size() / num_sequences));
}

test_suite* init_unit_test_suite(int, char* []) 
{
    test_suite* test = BOOST_TEST_SUITE("bzip2 test");
    test->add(BOOST_TEST_CASE(&bzip2_test));
    test->add(BOOST_TEST_CASE(&multiple_member_test));
    return test;
}
