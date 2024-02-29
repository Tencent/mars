// Copyright 2023 Andrey Semashev.
//
// Distributed under the Boost Software License, Version 1.0.
//
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// The test verifies that \c mars_boost::filesystem::path constructors
// from iterators don't interfere with initializer list constructor.
//
// https://github.com/boostorg/filesystem/issues/287

#include <boost/filesystem/path.hpp>
#include <boost/config.hpp>

#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)

#include <initializer_list>

void test(std::initializer_list< mars_boost::filesystem::path > paths)
{
}

void test(mars_boost::filesystem::path const& path)
{
}

int main()
{
    mars_boost::filesystem::path a, b;
    test({a, b});
}

#else // !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)

int main()
{
}

#endif // !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
