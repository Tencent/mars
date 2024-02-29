/*
Copyright 2012-2015 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/make_shared.hpp>

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
    try {
        mars_boost::make_shared<type[]>(6);
        BOOST_ERROR("make_shared did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::make_shared<type[][2]>(3);
        BOOST_ERROR("make_shared did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::make_shared<type[6]>();
        BOOST_ERROR("make_shared did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::make_shared<type[3][2]>();
        BOOST_ERROR("make_shared did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::make_shared_noinit<type[]>(6);
        BOOST_ERROR("make_shared_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::make_shared_noinit<type[][2]>(3);
        BOOST_ERROR("make_shared_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::make_shared_noinit<type[6]>();
        BOOST_ERROR("make_shared_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::make_shared_noinit<type[3][2]>();
        BOOST_ERROR("make_shared_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    return mars_boost::report_errors();
}
