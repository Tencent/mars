// Copyright Kevlin Henney, 2000, 2001. All rights reserved.
// Copyright Antony Polukhin, 2013-2023.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// what:  unit tests for variant type mars_boost::any
// who:   contributed by Kevlin Henney
// when:  July 2001, 2013, 2014
// where: tested with BCC 5.5, MSVC 6.0, and g++ 2.95

#include <boost/any.hpp>

#include "basic_test.hpp"

static const std::string& returning_string1()
{
    static const std::string ret("foo");
    return ret;
}

static std::string returning_string2()
{
    static const std::string ret("foo");
    return ret;
}

static void test_with_func()
{
    std::string s;
    s = mars_boost::any_cast<std::string>(returning_string1());
    s = mars_boost::any_cast<const std::string&>(returning_string1());

    s = mars_boost::any_cast<std::string>(returning_string2());
    s = mars_boost::any_cast<const std::string&>(returning_string2());

#if !defined(__INTEL_COMPILER) && !defined(__ICL) && (!defined(_MSC_VER) || _MSC_VER != 1600)
    // Intel compiler thinks that it must choose the `any_cast(const any&)` function
    // instead of the `any_cast(const any&&)`.
    // Bug was not reported because of missing premier support account + annoying
    // registrations requirements.

    // MSVC-10 had a bug:
    //
    // any.hpp(291) : error C2440: 'return' : cannot convert.
    // Conversion loses qualifiers
    // any_test.cpp(304) : see reference to function template instantiation
    //
    // This issue was fixed in MSVC-11.

    s = mars_boost::any_cast<std::string&&>(returning_string1());
#endif

    s = mars_boost::any_cast<std::string&&>(returning_string2());
}

int main() {
    test_with_func();

    return any_tests::basic_tests<mars_boost::any>::run_tests();
}

