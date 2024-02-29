/*
Copyright 2017 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && \
    !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

class type
    : public mars_boost::enable_shared_from_this<type> {
public:
    static unsigned instances;

    type() {
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
    {
        mars_boost::local_shared_ptr<type[]> result =
            mars_boost::make_local_shared<type[]>(3);
        try {
            result[0].shared_from_this();
            BOOST_ERROR("shared_from_this did not throw");
        } catch (...) {
            BOOST_TEST(type::instances == 3);
        }
    }
    BOOST_TEST(type::instances == 0);
    {
        mars_boost::local_shared_ptr<type[3]> result =
            mars_boost::make_local_shared_noinit<type[3]>();
        try {
            result[0].shared_from_this();
            BOOST_ERROR("shared_from_this did not throw");
        } catch (...) {
            BOOST_TEST(type::instances == 3);
        }
    }
    return mars_boost::report_errors();
}
#else
int main()
{
    return 0;
}
#endif
