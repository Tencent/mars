// Boost.Function library

//  Copyright 2016 Peter Dimov

//  Use, modification and distribution is subject to
//  the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/function.hpp>
#include <boost/core/lightweight_test_trait.hpp>

struct X
{
};

struct Y
{
};

struct Z
{
};

int main()
{
    typedef mars_boost::function<X(Y)> F1;

    BOOST_TEST_TRAIT_SAME(F1::result_type, X);
    BOOST_TEST_TRAIT_SAME(F1::argument_type, Y);

    typedef mars_boost::function<X(Y, Z)> F2;

    BOOST_TEST_TRAIT_SAME(F2::result_type, X);
    BOOST_TEST_TRAIT_SAME(F2::first_argument_type, Y);
    BOOST_TEST_TRAIT_SAME(F2::second_argument_type, Z);

    return mars_boost::report_errors();
}
