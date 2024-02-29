/*
Copyright 2014 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#if !defined(BOOST_NO_CXX11_SMART_PTR)
#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/make_unique.hpp>

struct type {
    int x;
    int y;
};

int main()
{
    {
        std::unique_ptr<type> result = mars_boost::make_unique<type>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result->x == 0);
        BOOST_TEST(result->y == 0);
    }
    {
        std::unique_ptr<const type> result =
            mars_boost::make_unique<const type>();
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result->x == 0);
        BOOST_TEST(result->y == 0);
    }
#if !defined(BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX)
    {
        std::unique_ptr<type> result =
            mars_boost::make_unique<type>({ 1, 2 });
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result->x == 1);
        BOOST_TEST(result->y == 2);
    }
    {
        std::unique_ptr<const type> result =
            mars_boost::make_unique<const type>({ 1, 2 });
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result->x == 1);
        BOOST_TEST(result->y == 2);
    }
#endif
    return mars_boost::report_errors();
}
#else
int main()
{
    return 0;
}
#endif
