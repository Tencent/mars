/*
Copyright 2012-2015 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/make_shared.hpp>

int main()
{
    {
        mars_boost::shared_ptr<int[]> result =
            mars_boost::make_shared<int[]>(4, 1);
        BOOST_TEST(result[0] == 1);
        BOOST_TEST(result[1] == 1);
        BOOST_TEST(result[2] == 1);
        BOOST_TEST(result[3] == 1);
    }
    {
        mars_boost::shared_ptr<int[4]> result =
            mars_boost::make_shared<int[4]>(1);
        BOOST_TEST(result[0] == 1);
        BOOST_TEST(result[1] == 1);
        BOOST_TEST(result[2] == 1);
        BOOST_TEST(result[3] == 1);
    }
    {
        mars_boost::shared_ptr<const int[]> result =
            mars_boost::make_shared<const int[]>(4, 1);
        BOOST_TEST(result[0] == 1);
        BOOST_TEST(result[1] == 1);
        BOOST_TEST(result[2] == 1);
        BOOST_TEST(result[3] == 1);
    }
    {
        mars_boost::shared_ptr<const int[4]> result =
            mars_boost::make_shared<const int[4]>(1);
        BOOST_TEST(result[0] == 1);
        BOOST_TEST(result[1] == 1);
        BOOST_TEST(result[2] == 1);
        BOOST_TEST(result[3] == 1);
    }
    return mars_boost::report_errors();
}
