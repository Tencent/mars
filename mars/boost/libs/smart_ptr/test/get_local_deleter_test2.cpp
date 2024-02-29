//
// get_local_deleter_test2.cpp
//
// Copyright 2017 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

struct deleter;

struct X
{
};

static void test_lsp_get_deleter( mars_boost::local_shared_ptr<X> const & p )
{
    BOOST_TEST( mars_boost::get_deleter<deleter>( p ) != 0 );
}

static void test_sp_get_deleter( mars_boost::shared_ptr<X> const & p )
{
    BOOST_TEST( mars_boost::get_deleter<deleter>( p ) != 0 );
}

struct deleter
{
    void operator()( X const * p ) { delete p; }
};

int main()
{
    mars_boost::local_shared_ptr<X> p( new X, deleter() );

    test_lsp_get_deleter( p );
    test_sp_get_deleter( p );

    return mars_boost::report_errors();
}
