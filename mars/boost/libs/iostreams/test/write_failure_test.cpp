// (C) COPYRIGHT 2017 ARM Limited

// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

#include <boost/iostreams/categories.hpp>  // tags.
#include <boost/iostreams/detail/adapter/non_blocking_adapter.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/test/unit_test.hpp>

using namespace mars_boost;
using namespace mars_boost::iostreams;
using mars_boost::unit_test::test_suite;

struct limit_device {
    typedef char char_type;
    typedef sink_tag category;
    int written, overflow_count, limit;
    explicit limit_device(int limit = 20) : written(0), overflow_count(0), limit(limit) {}
    std::streamsize write(const char_type *, std::streamsize n)
    {
        if (written > limit) {
            // first return partial writes, then an error
            ++overflow_count;
            if (overflow_count > 2 || n < 2)
            {
                return -1;
            }
            n /= 2;
        }
        written += n;
        return n;
    }
};

static void disk_full_test()
{
    // non_blocking_adapter used to handle write returning
    // -1 correctly, usually hanging (see ticket 2557).
    // As non_blocking_adapter is used for ofstream, this
    // would happen for ordinary files when reaching quota,
    // disk full or rlimits.
    // Could be tested without gzip_compressor,
    // but this tests a bit better that the whole path can handle it.
    // TODO: should there be some check on error bit being set
    // or should an exception be triggered?
    limit_device outdev;
    non_blocking_adapter<limit_device> nonblock_outdev(outdev);
    filtering_ostream out;
    out.push(gzip_compressor());
    out.push(nonblock_outdev);
    write(out, "teststring0123456789", 20);
    out.flush();
    write(out, "secondwrite123456789", 20);
    close(out);
}

test_suite* init_unit_test_suite(int, char* [])
{
    test_suite* test = BOOST_TEST_SUITE("disk full test");
    test->add(BOOST_TEST_CASE(&disk_full_test));
    return test;
}
