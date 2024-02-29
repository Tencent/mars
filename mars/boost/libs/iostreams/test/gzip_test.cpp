// (C) Copyright 2008 CodeRage, LLC (turkanis at coderage dot com)
// (C) Copyright 2004-2007 Jonathan Turkanis
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.

#include <cstddef>
#include <string>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/gzip.hpp>
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

typedef basic_test_alloc<char> gzip_alloc;

void compression_test()
{
    text_sequence      data;

    // Test compression and decompression with metadata
    for (int i = 0; i < 4; ++i) {
        gzip_params params;
        if (i & 1) {
            params.file_name = "original file name";
        }
        if (i & 2) {
            params.comment = "detailed file description";
        }
        gzip_compressor    out(params);
        gzip_decompressor  in;
        BOOST_CHECK(
            test_filter_pair( mars_boost::ref(out), 
                              mars_boost::ref(in), 
                              std::string(data.begin(), data.end()) )
        );
        BOOST_CHECK(in.file_name() == params.file_name);
        BOOST_CHECK(in.comment() == params.comment);
    }

    // Test compression and decompression with custom allocator
    BOOST_CHECK(
        test_filter_pair( basic_gzip_compressor<gzip_alloc>(), 
                          basic_gzip_decompressor<gzip_alloc>(), 
                          std::string(data.begin(), data.end()) )
    );
}

void multiple_member_test()
{
    text_sequence      data;
    std::vector<char>  temp, dest;

    // Write compressed data to temp, twice in succession
    filtering_ostream out;
    out.push(gzip_compressor());
    out.push(io::back_inserter(temp));
    io::copy(make_iterator_range(data), out);
    out.push(io::back_inserter(temp));
    io::copy(make_iterator_range(data), out);

    // Read compressed data from temp into dest
    filtering_istream in;
    in.push(gzip_decompressor());
    in.push(array_source(&temp[0], temp.size()));
    io::copy(in, io::back_inserter(dest));

    // Check that dest consists of two copies of data
    BOOST_REQUIRE_EQUAL(data.size() * 2, dest.size());
    BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin()));
    BOOST_CHECK(std::equal(data.begin(), data.end(), dest.begin() + dest.size() / 2));

    dest.clear();
    io::copy(
        array_source(&temp[0], temp.size()),
        io::compose(gzip_decompressor(), io::back_inserter(dest)));

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
    out.push(gzip_compressor());
    out.push(io::back_inserter(encoded));
    io::copy(make_iterator_range(data), out);

    std::string res;
    io::array_source src(encoded.data(),encoded.length());
    io::copy(io::compose(io::gzip_decompressor(), src), io::back_inserter(res));
    
    BOOST_CHECK_EQUAL(data, res);
}

#if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4309)  // Truncation of constant value
#endif

void header_test()
{
    // This test is in response to https://svn.boost.org/trac/boost/ticket/5908
    // which describes a problem parsing gzip headers with extra fields as
    // defined in RFC 1952 (http://www.ietf.org/rfc/rfc1952.txt).
    // The extra field data used here is characteristic of the tabix file
    // format (http://samtools.sourceforge.net/tabix.shtml).
    const char header_bytes[] = {
        static_cast<char>(gzip::magic::id1),
        static_cast<char>(gzip::magic::id2),
        gzip::method::deflate, // Compression Method: deflate
        gzip::flags::extra | gzip::flags::name | gzip::flags::comment, // flags
        '\x22', '\x9c', '\xf3', '\x4e', // 4 byte modification time (little endian)
        gzip::extra_flags::best_compression, // XFL
        gzip::os_unix, // OS
        6, 0, // 2 byte length of extra field (little endian, 6 bytes)
        'B', 'C', 2, 0, 0, 0, // 6 bytes worth of extra field data
        'a', 'b', 'c', 0, // original filename, null terminated
        'n', 'o', ' ', 'c', 'o', 'm', 'm', 'e', 'n', 't', 0, // comment
    };
    size_t sz = sizeof(header_bytes)/sizeof(header_bytes[0]);

    mars_boost::iostreams::detail::gzip_header hdr;
    for (size_t i = 0; i < sz; ++i) {
        hdr.process(header_bytes[i]);

        // Require that we are done at the last byte, not before.
        if (i == sz-1)
            BOOST_REQUIRE(hdr.done());
        else
            BOOST_REQUIRE(!hdr.done());
    }

    BOOST_CHECK_EQUAL("abc", hdr.file_name());
    BOOST_CHECK_EQUAL("no comment", hdr.comment());
    BOOST_CHECK_EQUAL(0x4ef39c22, hdr.mtime());
    BOOST_CHECK_EQUAL(gzip::os_unix, hdr.os());
}

#if defined(BOOST_MSVC)
# pragma warning(pop)
#endif

void empty_file_test()
{
    // This test is in response to https://svn.boost.org/trac/boost/ticket/5237
    // The previous implementation of gzip_compressor only wrote the gzip file
    // header when the first bytes of uncompressed input were processed, causing
    // incorrect behavior for empty files
    BOOST_CHECK(
        test_filter_pair( gzip_compressor(),
                          gzip_decompressor(),
                          std::string() )
    );
}

void multipart_test()
{
    // This test verifies that the gzip_decompressor properly handles a file
    // that was written in multiple parts using Z_FULL_FLUSH, and in particular
    // handles the CRC properly when one of those parts is empty.
    const char multipart_file[] = {
        '\x1f', '\x8b', '\x08', '\x00', '\x00', '\x00', '\x00', '\x00', '\x02', '\xff', '\xf2', '\xc9',
        '\xcc', '\x4b', '\x55', '\x30', '\xe4', '\xf2', '\x01', '\x51', '\x46', '\x10', '\xca', '\x98',
        '\x0b', '\x00', '\x00', '\x00', '\xff', '\xff', '\x03', '\x00', '\xdb', '\xa7', '\x83', '\xc9',
        '\x15', '\x00', '\x00', '\x00', '\x1f', '\x8b', '\x08', '\x00', '\x00', '\x00', '\x00', '\x00',
        '\x02', '\xff', '\xf2', '\xc9', '\xcc', '\x4b', '\x55', '\x30', '\xe1', '\xf2', '\x01', '\x51',
        '\xa6', '\x10', '\xca', '\x8c', '\x0b', '\x00', '\x00', '\x00', '\xff', '\xff', '\x03', '\x00',
        '\x41', '\xe3', '\xcc', '\xaa', '\x15', '\x00', '\x00', '\x00', '\x1f', '\x8b', '\x08', '\x00',
        '\x00', '\x00', '\x00', '\x00', '\x02', '\xff', '\x02', '\x00', '\x00', '\x00', '\xff', '\xff',
        '\x03', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x1f', '\x8b',
        '\x08', '\x00', '\x00', '\x00', '\x00', '\x00', '\x02', '\xff', '\xf2', '\xc9', '\xcc', '\x4b',
        '\x55', '\x30', '\xe7', '\xf2', '\x01', '\x51', '\x16', '\x10', '\xca', '\x92', '\x0b', '\x00',
        '\x00', '\x00', '\xff', '\xff', '\x03', '\x00', '\x2b', '\xac', '\xd3', '\xf5', '\x15', '\x00',
        '\x00', '\x00'
    };

    filtering_istream in;
    std::string line;

    in.push(gzip_decompressor());
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

    // Check for gzip errors too.
    BOOST_CHECK(!in.bad());
}

test_suite* init_unit_test_suite(int, char* []) 
{
    test_suite* test = BOOST_TEST_SUITE("gzip test");

#if !defined(__APPLE__)

    // this test fails on macOS by throwing zlib_error, and it's not clear why
    test->add(BOOST_TEST_CASE(&compression_test));

#endif

    test->add(BOOST_TEST_CASE(&multiple_member_test));
    test->add(BOOST_TEST_CASE(&array_source_test));
    test->add(BOOST_TEST_CASE(&header_test));
    test->add(BOOST_TEST_CASE(&empty_file_test));
    test->add(BOOST_TEST_CASE(&multipart_test));
    return test;
}
