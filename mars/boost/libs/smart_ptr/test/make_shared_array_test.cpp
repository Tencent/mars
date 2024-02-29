/*
Copyright 2012-2015 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/align/is_aligned.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/type_traits/alignment_of.hpp>

class type {
public:
    static unsigned instances;

    type()
        : value_(0.0) {
        ++instances;
    }

    ~type() {
        --instances;
    }

    void set(long double value) {
        value_ = value;
    }

    long double get() const {
        return value_;
    }

private:
    type(const type&);
    type& operator=(const type&);

    long double value_;
};

unsigned type::instances = 0;

int main()
{
    {
        mars_boost::shared_ptr<int[]> result =
            mars_boost::make_shared<int[]>(3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
        BOOST_TEST(result[0] == 0);
        BOOST_TEST(result[1] == 0);
        BOOST_TEST(result[2] == 0);
    }
    {
        mars_boost::shared_ptr<int[3]> result =
            mars_boost::make_shared<int[3]>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
        BOOST_TEST(result[0] == 0);
        BOOST_TEST(result[1] == 0);
        BOOST_TEST(result[2] == 0);
    }
    {
        mars_boost::shared_ptr<int[][2]> result =
            mars_boost::make_shared<int[][2]>(2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 0);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 0);
    }
    {
        mars_boost::shared_ptr<int[2][2]> result =
            mars_boost::make_shared<int[2][2]>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 0);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 0);
    }
    {
        mars_boost::shared_ptr<const int[]> result =
            mars_boost::make_shared<const int[]>(3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
        BOOST_TEST(result[0] == 0);
        BOOST_TEST(result[1] == 0);
        BOOST_TEST(result[2] == 0);
    }
    {
        mars_boost::shared_ptr<const int[3]> result =
            mars_boost::make_shared<const int[3]>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
        BOOST_TEST(result[0] == 0);
        BOOST_TEST(result[1] == 0);
        BOOST_TEST(result[2] == 0);
    }
    {
        mars_boost::shared_ptr<const int[][2]> result =
            mars_boost::make_shared<const int[][2]>(2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 0);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 0);
    }
    {
        mars_boost::shared_ptr<const int[2][2]> result =
            mars_boost::make_shared<const int[2][2]>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 0);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 0);
    }
    {
        mars_boost::shared_ptr<type[]> result =
            mars_boost::make_shared<type[]>(3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 3);
        mars_boost::weak_ptr<type[]> w1 = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<type[3]> result =
            mars_boost::make_shared<type[3]>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 3);
        mars_boost::weak_ptr<type[3]> w1 = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<type[][2]> result =
            mars_boost::make_shared<type[][2]>(2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<type[2][2]> result =
            mars_boost::make_shared<type[2][2]>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<const type[]> result =
            mars_boost::make_shared<const type[]>(3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<const type[3]> result =
            mars_boost::make_shared<const type[3]>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<const type[][2]> result =
            mars_boost::make_shared<const type[][2]>(2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<const type[2][2]> result =
            mars_boost::make_shared<const type[2][2]>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    return mars_boost::report_errors();
}
