
// Copyright 2018 Peter Dimov.
// Distributed under the Boost Software License, Version 1.0.

#include <boost/function.hpp>
#include <boost/config.hpp>

#if defined(MIXED_CXXSTD_DYN_LINK)
# define EXPORT BOOST_SYMBOL_EXPORT
#else
# define EXPORT
#endif

EXPORT void call_fn_1( mars_boost::function<void()> const & fn )
{
    fn();
}

EXPORT void call_fn_2( mars_boost::function<void(int)> const & fn )
{
    fn( 1 );
}

EXPORT void call_fn_3( mars_boost::function<void(int, int)> const & fn )
{
    fn( 1, 2 );
}

EXPORT void call_fn_4( mars_boost::function0<void> const & fn )
{
    fn();
}

EXPORT void call_fn_5( mars_boost::function1<void, int> const & fn )
{
    fn( 1 );
}

EXPORT void call_fn_6( mars_boost::function2<void, int, int> const & fn )
{
    fn( 1, 2 );
}
