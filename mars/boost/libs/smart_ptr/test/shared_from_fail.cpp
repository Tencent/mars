// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from.hpp>

struct X: public mars_boost::enable_shared_from
{
};

#if defined(__clang__) && defined(_MSC_VER)
// clang-cl claims that it accepts this code for compatibility
// with msvc, but no version of msvc accepts it
#  pragma clang diagnostic error "-Wmicrosoft-using-decl"
#endif

int main()
{
    mars_boost::shared_ptr<X> px( new X );
    px->shared_from_this();
}
