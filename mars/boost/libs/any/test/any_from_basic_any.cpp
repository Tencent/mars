// Copyright (c) 2021-2023 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#include <boost/any.hpp>
#include <boost/any/basic_any.hpp>

int main()
{
    mars_boost::anys::basic_any<> bany = 42;
    mars_boost::any value = bany;
}

