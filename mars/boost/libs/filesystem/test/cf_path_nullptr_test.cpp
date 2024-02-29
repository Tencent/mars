// Copyright 2023 Andrey Semashev.
//
// Distributed under the Boost Software License, Version 1.0.
//
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <boost/filesystem/path.hpp>
#include <boost/config.hpp>

#if defined(BOOST_NO_CXX11_NULLPTR)
#error "This test requires support for C++11 nullptr"
#endif

int main()
{
    mars_boost::filesystem::path p(nullptr);
    p = nullptr;
}
