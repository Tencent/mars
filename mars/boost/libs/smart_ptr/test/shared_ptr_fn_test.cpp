//
// shared_ptr_fn_test.cpp
//
// Copyright 2017 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

static void f()
{
}

struct null_deleter
{
    template<class Y> void operator()( Y* ) {}
};

int main()
{
    mars_boost::shared_ptr<void()> pf( f, null_deleter() );

    BOOST_TEST( pf.get() == f );
    BOOST_TEST_EQ( pf.use_count(), 1 );
    BOOST_TEST( mars_boost::get_deleter<null_deleter>( pf ) != 0 );

    mars_boost::weak_ptr<void()> wp( pf );

    BOOST_TEST( wp.lock().get() == f );
    BOOST_TEST_EQ( wp.use_count(), 1 );

    pf.reset();

    BOOST_TEST( wp.lock().get() == 0 );
    BOOST_TEST_EQ( wp.use_count(), 0 );

    return mars_boost::report_errors();
}
