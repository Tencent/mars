// (C) Copyright 2018 Mario Suvajac
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.

#include <boost/iostreams/detail/adapter/non_blocking_adapter.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/iostreams/categories.hpp>

#include <algorithm>

// Source that reads only one byte every time read() is called.
class read_one_source
{
public:
    typedef char                         char_type;
    typedef mars_boost::iostreams::source_tag category;

    template <std::size_t N>
    read_one_source(const char (&data)[N])
        : data_size_m(N), data_m(data), pos_m(0)
    {
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if (pos_m < data_size_m && n > 0)
        {
            *s = data_m[pos_m++];
            return 1;
        }
        else
        {
            return -1;
        }
    }

private:
    std::size_t data_size_m;
    const char* data_m;
    std::size_t pos_m;
};

void nonblocking_read_test()
{
    static const int data_size_k = 100;

    char data[data_size_k];
    std::copy(mars_boost::counting_iterator<char>(0),
              mars_boost::counting_iterator<char>(data_size_k),
              data);

    read_one_source src(data);
    mars_boost::iostreams::non_blocking_adapter<read_one_source> nb(src);

    char read_data[data_size_k];
    std::streamsize amt = mars_boost::iostreams::read(nb, read_data, data_size_k);

    BOOST_CHECK_EQUAL(amt, data_size_k);

    for (int i = 0; i < data_size_k; ++i)
    {
        BOOST_CHECK_EQUAL(std::char_traits<char>::to_int_type(read_data[i]), i);
    }
}

mars_boost::unit_test::test_suite* init_unit_test_suite(int, char* [])
{
    mars_boost::unit_test::test_suite* test = BOOST_TEST_SUITE("non-blocking read test");
    test->add(BOOST_TEST_CASE(&nonblocking_read_test));
    return test;
}
