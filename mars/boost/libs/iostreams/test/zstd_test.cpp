// (C) COPYRIGHT 2018 Reimar Döffinger
// Based on zstd_test.cpp by:
// (C) COPYRIGHT 2017 ARM Limited
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
#include <boost/iostreams/filter/zstd.hpp>
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

typedef basic_test_alloc<char> zstd_alloc;

void compression_test()
{
    text_sequence      data;

    // Test compression and decompression with custom allocator
    BOOST_CHECK(
        test_filter_pair( basic_zstd_compressor<zstd_alloc>(),
                          basic_zstd_decompressor<zstd_alloc>(),
                          std::string(data.begin(), data.end()) )
    );
}

void multiple_member_test()
{
    text_sequence      data;
    std::vector<char>  temp, dest;

    // Write compressed data to temp, twice in succession
    filtering_ostream out;
    out.push(zstd_compressor());
    out.push(io::back_inserter(temp));
    io::copy(make_iterator_range(data), out);
    out.push(io::back_inserter(temp));
    io::copy(make_iterator_range(data), out);
    BOOST_CHECK(std::equal(temp.begin(), temp.begin() + temp.size()/2, temp.begin() + temp.size()/2));

    // Read compressed data from temp into dest
    filtering_istream in;
    in.push(zstd_decompressor());
    in.push(array_source(&temp[0], temp.size()));
    io::copy(in, io::back_inserter(dest));

    // Check that dest consists of two copies of data
    BOOST_REQUIRE_EQUAL(data.size() * 2, dest.size());
    BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin()));
    BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin() + dest.size() / 2));

    dest.clear();
    io::copy(
        array_source(&temp[0], temp.size()),
        io::compose(zstd_decompressor(), io::back_inserter(dest)));

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
    out.push(zstd_compressor());
    out.push(io::back_inserter(encoded));
    io::copy(make_iterator_range(data), out);

    std::string res;
    io::array_source src(encoded.data(),encoded.length());
    io::copy(io::compose(io::zstd_decompressor(), src), io::back_inserter(res));

    BOOST_CHECK_EQUAL(data, res);
}

void empty_file_test()
{
    // This test is in response to https://svn.boost.org/trac/boost/ticket/5237
    // The previous implementation of gzip_compressor only wrote the gzip file
    // header when the first bytes of uncompressed input were processed, causing
    // incorrect behavior for empty files
    BOOST_CHECK(
        test_filter_pair( zstd_compressor(),
                          zstd_decompressor(),
                          std::string() )
    );
}

void multipart_test()
{
    // This test verifies that the zstd_decompressor properly handles a file
    // that consists of multiple concatenated files (matches unzstd behaviour)
    static const char multipart_file[] = {
        '\x28', '\xb5', '\x2f', '\xfd', '\x24', '\x15', '\x95', '\x00', '\x00', '\x50', '\x4c', '\x69',
        '\x6e', '\x65', '\x20', '\x31', '\x0a', '\x32', '\x33', '\x0a', '\x02', '\x00', '\x60', '\x84',
        '\xae', '\x62', '\x04', '\x19', '\xf8', '\xe1', '\x2d', '\x28', '\xb5', '\x2f', '\xfd', '\x24',
        '\x15', '\x95', '\x00', '\x00', '\x50', '\x4c', '\x69', '\x6e', '\x65', '\x20', '\x34', '\x0a',
        '\x35', '\x36', '\x0a', '\x02', '\x00', '\x60', '\x84', '\xae', '\x62', '\x04', '\x5f', '\xcf',
        '\xd5', '\xb8', '\x28', '\xb5', '\x2f', '\xfd', '\x24', '\x00', '\x01', '\x00', '\x00', '\x99',
        '\xe9', '\xd8', '\x51', '\x28', '\xb5', '\x2f', '\xfd', '\x24', '\x15', '\x95', '\x00', '\x00',
        '\x50', '\x4c', '\x69', '\x6e', '\x65', '\x20', '\x37', '\x0a', '\x38', '\x39', '\x0a', '\x02',
        '\x00', '\x60', '\x84', '\xae', '\x62', '\x04', '\x94', '\x13', '\xdb', '\xae'
    };

    filtering_istream in;
    std::string line;

    in.push(zstd_decompressor());
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

    // Check for zstd errors too.
    BOOST_CHECK(!in.bad());
}

test_suite* init_unit_test_suite(int, char* [])
{
    test_suite* test = BOOST_TEST_SUITE("zstd test");
    test->add(BOOST_TEST_CASE(&compression_test));
    test->add(BOOST_TEST_CASE(&multiple_member_test));
    test->add(BOOST_TEST_CASE(&array_source_test));
    test->add(BOOST_TEST_CASE(&empty_file_test));
    test->add(BOOST_TEST_CASE(&multipart_test));
    return test;
}
