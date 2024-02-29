// Copyright 2006 Alexander Nasonov.
// Copyright Antony Polukhin, 2013-2023.
// Copyright Ruslan Arutyunyan, 2019-2021.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/any/basic_any.hpp>

int main() {
    mars_boost::anys::basic_any<> const a;
    mars_boost::any_cast<int&>(a);
}

