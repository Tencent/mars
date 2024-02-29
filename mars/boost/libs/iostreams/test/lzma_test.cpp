// (C) COPYRIGHT 2017 ARM Limited
// Based on gzip_test.cpp by:
// (C) Copyright 2008 CodeRage, LLC (turkanis at coderage dot com)
// (C) Copyright 2004-2007 Jonathan Turkanis
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.

// Note: basically a copy-paste of the gzip test

#include <cstddef>
#include <string>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filter/test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/ref.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "detail/sequence.hpp"
#include "detail/verification.hpp"

using namespace mars_boost;
using namespace mars_boost::iostreams;
using namespace mars_boost::iostreams::test;
namespace io = mars_boost::iostreams;
using mars_boost::unit_test::test_suite;

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

typedef basic_test_alloc<char> lzma_alloc;

void compression_test()
{
    text_sequence      data;

    // Test compression and decompression with custom allocator
    BOOST_CHECK(
        test_filter_pair( basic_lzma_compressor<lzma_alloc>(),
                          basic_lzma_decompressor<lzma_alloc>(),
                          std::string(data.begin(), data.end()) )
    );
}

void multiple_member_test()
{
    text_sequence      data;
    std::vector<char>  temp, dest;

    // Write compressed data to temp, twice in succession
    filtering_ostream out;
    out.push(lzma_compressor());
    out.push(io::back_inserter(temp));
    io::copy(make_iterator_range(data), out);
    out.push(io::back_inserter(temp));
    io::copy(make_iterator_range(data), out);

    // Read compressed data from temp into dest
    filtering_istream in;
    in.push(lzma_decompressor());
    in.push(array_source(&temp[0], temp.size()));
    io::copy(in, io::back_inserter(dest));

    // Check that dest consists of two copies of data
    BOOST_REQUIRE_EQUAL(data.size() * 2, dest.size());
    BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin()));
    BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin() + dest.size() / 2));

    dest.clear();
    io::copy(
        array_source(&temp[0], temp.size()),
        io::compose(lzma_decompressor(), io::back_inserter(dest)));

    // Check that dest consists of two copies of data
    BOOST_REQUIRE_EQUAL(data.size() * 2, dest.size());
    BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin()));
    BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin() + dest.size() / 2));
}

void array_source_test()
{
    std::string data = "simple test string.";
    std::string encoded;

    filtering_ostream out;
    out.push(lzma_compressor());
    out.push(io::back_inserter(encoded));
    io::copy(make_iterator_range(data), out);

    std::string res;
    io::array_source src(encoded.data(),encoded.length());
    io::copy(io::compose(io::lzma_decompressor(), src), io::back_inserter(res));

    BOOST_CHECK_EQUAL(data, res);
}

void empty_file_test()
{
    // This test is in response to https://svn.boost.org/trac/boost/ticket/5237
    // The previous implementation of gzip_compressor only wrote the gzip file
    // header when the first bytes of uncompressed input were processed, causing
    // incorrect behavior for empty files
    BOOST_CHECK(
        test_filter_pair( lzma_compressor(),
                          lzma_decompressor(),
                          std::string() )
    );
}

void multipart_test()
{
    // This test verifies that the lzma_decompressor properly handles a file
    // that consists of multiple concatenated files (matches unxz behaviour)
    static const char multipart_file[] = {
        '\xfd', '\x37', '\x7a', '\x58', '\x5a', '\x00', '\x00', '\x04', '\xe6', '\xd6', '\xb4', '\x46',
        '\x02', '\x00', '\x21', '\x01', '\x1c', '\x00', '\x00', '\x00', '\x10', '\xcf', '\x58', '\xcc',
        '\xe0', '\x00', '\x14', '\x00', '\x11', '\x5d', '\x00', '\x26', '\x1a', '\x49', '\xc6', '\x67',
        '\x41', '\x3f', '\x96', '\x8c', '\x25', '\x02', '\xb3', '\x4d', '\x16', '\xa8', '\xb4', '\x40',
        '\x00', '\x00', '\x00', '\x00', '\xeb', '\xad', '\x3f', '\xbf', '\x8c', '\x8c', '\x72', '\x25',
        '\x00', '\x01', '\x2d', '\x15', '\x2f', '\x0b', '\x71', '\x6d', '\x1f', '\xb6', '\xf3', '\x7d',
        '\x01', '\x00', '\x00', '\x00', '\x00', '\x04', '\x59', '\x5a', '\xfd', '\x37', '\x7a', '\x58',
        '\x5a', '\x00', '\x00', '\x04', '\xe6', '\xd6', '\xb4', '\x46', '\x02', '\x00', '\x21', '\x01',
        '\x1c', '\x00', '\x00', '\x00', '\x10', '\xcf', '\x58', '\xcc', '\xe0', '\x00', '\x14', '\x00',
        '\x11', '\x5d', '\x00', '\x26', '\x1a', '\x49', '\xc6', '\x67', '\x41', '\x4d', '\x84', '\x0c',
        '\x25', '\x1f', '\x5e', '\x1d', '\x4a', '\x91', '\x61', '\xa0', '\x00', '\x00', '\x00', '\x00',
        '\x56', '\x76', '\x71', '\xf0', '\x54', '\x21', '\xa2', '\x5b', '\x00', '\x01', '\x2d', '\x15',
        '\x2f', '\x0b', '\x71', '\x6d', '\x1f', '\xb6', '\xf3', '\x7d', '\x01', '\x00', '\x00', '\x00',
        '\x00', '\x04', '\x59', '\x5a', '\xfd', '\x37', '\x7a', '\x58', '\x5a', '\x00', '\x00', '\x04',
        '\xe6', '\xd6', '\xb4', '\x46', '\x00', '\x00', '\x00', '\x00', '\x1c', '\xdf', '\x44', '\x21',
        '\x1f', '\xb6', '\xf3', '\x7d', '\x01', '\x00', '\x00', '\x00', '\x00', '\x04', '\x59', '\x5a',
        '\xfd', '\x37', '\x7a', '\x58', '\x5a', '\x00', '\x00', '\x04', '\xe6', '\xd6', '\xb4', '\x46',
        '\x02', '\x00', '\x21', '\x01', '\x1c', '\x00', '\x00', '\x00', '\x10', '\xcf', '\x58', '\xcc',
        '\xe0', '\x00', '\x14', '\x00', '\x11', '\x5d', '\x00', '\x26', '\x1a', '\x49', '\xc6', '\x67',
        '\x41', '\x5b', '\x71', '\x8c', '\x25', '\x3c', '\x08', '\xec', '\x79', '\xa7', '\x7b', '\x60',
        '\x00', '\x00', '\x00', '\x00', '\xc7', '\x62', '\xbb', '\xaa', '\x59', '\x96', '\x2b', '\xa4',
        '\x00', '\x01', '\x2d', '\x15', '\x2f', '\x0b', '\x71', '\x6d', '\x1f', '\xb6', '\xf3', '\x7d',
        '\x01', '\x00', '\x00', '\x00', '\x00', '\x04', '\x59', '\x5a'
    };

    filtering_istream in;
    std::string line;

    in.push(lzma_decompressor());
    in.push(io::array_source(multipart_file, sizeof(multipart_file)));

    // First part
    std::getline(in, line);
    BOOST_CHECK_EQUAL("Line 1", line);
    std::getline(in, line);
    BOOST_CHECK_EQUAL("Line 2", line);
    std::getline(in, line);
    BOOST_CHECK_EQUAL("Line 3", line);

    // Second part immediately follows
    std::getline(in, line);
    BOOST_CHECK_EQUAL("Line 4", line);
    std::getline(in, line);
    BOOST_CHECK_EQUAL("Line 5", line);
    std::getline(in, line);
    BOOST_CHECK_EQUAL("Line 6", line);

    // Then an empty part, followed by one last 3-line part.
    std::getline(in, line);
    BOOST_CHECK_EQUAL("Line 7", line);
    std::getline(in, line);
    BOOST_CHECK_EQUAL("Line 8", line);
    std::getline(in, line);
    BOOST_CHECK_EQUAL("Line 9", line);

    // Check for lzma errors too.
    BOOST_CHECK(!in.bad());
}

void multithreaded_test()
{
    text_sequence      data;

    // Get correct compressed string at level 2.
    // Tests legacy capability of providing a single integer to the
    // lzma_compressor constructor to be used as the "level" to initialize
    // lzma_params.
    std::string  correct_level_2;
    {
        filtering_ostream out;
        out.push(lzma_compressor(2));
        out.push(io::back_inserter(correct_level_2));
        io::copy(make_iterator_range(data), out);
    }

    // Tests omitting the threads parameters and arriving at same compressed data.
    BOOST_CHECK(
        test_output_filter( lzma_compressor(lzma_params(2)),
                            std::string(data.begin(), data.end()),
                            correct_level_2 )
    );

    // Test specifying a single thread and arriving at same compressed data.
    BOOST_CHECK(
        test_output_filter( lzma_compressor(lzma_params(2, 1)),
                            std::string(data.begin(), data.end()),
                            correct_level_2 )
    );

    // Test specifying multiple threads and arriving at same compressed data.
    BOOST_CHECK(
        test_output_filter( lzma_compressor(lzma_params(2, 4)),
                            std::string(data.begin(), data.end()),
                            correct_level_2 )
    );

    // Test specifying "0" threads, which is interpreted as
    // using all cores, or 1 thread if such capability is missing.
    BOOST_CHECK(
        test_output_filter( lzma_compressor(lzma_params(2, 0)),
                            std::string(data.begin(), data.end()),
                            correct_level_2 )
    );

    // Test that decompressor works to decompress the output with various thread values.
    // Threading shouldn't affect the decompression and, in fact, isn't
    // threaded in current implementation of liblzma. Both the level and
    // threads options are ignored by the decompressor.
    BOOST_CHECK(
        test_input_filter( lzma_decompressor(lzma_params(2, 1)),
                            correct_level_2,
                            std::string(data.begin(), data.end()) )
    );
    BOOST_CHECK(
        test_input_filter( lzma_decompressor(lzma_params(2, 4)),
                            correct_level_2,
                            std::string(data.begin(), data.end()) )
    );
    BOOST_CHECK(
        test_input_filter( lzma_decompressor(lzma_params(2, 0)),
                            correct_level_2,
                            std::string(data.begin(), data.end()) )
    );

}

test_suite* init_unit_test_suite(int, char* []) 
{
    test_suite* test = BOOST_TEST_SUITE("lzma test");
    test->add(BOOST_TEST_CASE(&compression_test));
    test->add(BOOST_TEST_CASE(&multiple_member_test));
    test->add(BOOST_TEST_CASE(&array_source_test));
    test->add(BOOST_TEST_CASE(&empty_file_test));
    test->add(BOOST_TEST_CASE(&multipart_test));
    test->add(BOOST_TEST_CASE(&multithreaded_test));
    return test;
}
