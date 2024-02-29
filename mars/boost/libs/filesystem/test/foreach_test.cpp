
// Copyright 2018 Peter Dimov.
//
// Distributed under the Boost Software License, Version 1.0.
//
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

// See library home page at http://www.boost.org/libs/filesystem

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/config.hpp>

namespace fs = mars_boost::filesystem;

int main()
{
    {
        fs::directory_iterator const it;

        BOOST_FOREACH(fs::path const& p, it)
        {
            p.string();
        }
    }

#if !defined(BOOST_NO_CXX11_RANGE_BASED_FOR)

    {
        fs::directory_iterator const it;

        for (fs::path const& p : it)
        {
            p.string();
        }
    }

#endif

    {
        fs::recursive_directory_iterator it;

        BOOST_FOREACH(fs::path const& p, it)
        {
            p.string();
        }
    }

#if !defined(BOOST_NO_CXX11_RANGE_BASED_FOR)

    {
        fs::recursive_directory_iterator const it;

        for (fs::path const& p : it)
        {
            p.string();
        }
    }

#endif
}
