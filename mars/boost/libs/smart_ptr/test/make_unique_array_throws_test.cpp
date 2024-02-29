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

class type {
public:
    static unsigned instances;

    type() {
        if (instances == 5) {
            throw true;
        }
        ++instances;
    }

    ~type() {
        --instances;
    }

private:
    type(const type&);
    type& operator=(const type&);
};

unsigned type::instances = 0;

int main()
{
    BOOST_TEST(type::instances == 0);
    try {
        mars_boost::make_unique<type[]>(6);
        BOOST_ERROR("make_unique did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    BOOST_TEST(type::instances == 0);
    try {
        mars_boost::make_unique<type[][2]>(3);
        BOOST_ERROR("make_unique did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    BOOST_TEST(type::instances == 0);
    try {
        mars_boost::make_unique_noinit<type[]>(6);
        BOOST_ERROR("make_unique_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    BOOST_TEST(type::instances == 0);
    try {
        mars_boost::make_unique_noinit<type[][2]>(3);
        BOOST_ERROR("make_unique_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    return mars_boost::report_errors();
}
#else
int main()
{
    return 0;
}
#endif
