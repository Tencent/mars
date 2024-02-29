
// shared_from_test.cpp
//
// Copyright 2019 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/smart_ptr/enable_shared_from.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>

//

class X
{
private:

    int m_;

public:

    X(): m_() {}
};

class Y: public mars_boost::enable_shared_from
{
};

class Z: public X, public Y
{
public:

    mars_boost::shared_ptr<Z> shared_from_this()
    {
        return mars_boost::shared_from( this );
    }
};

void null_deleter( void const* )
{
}

int main()
{
    mars_boost::shared_ptr<Z> p( new Z );

    try
    {
        mars_boost::shared_ptr<Z> q = p->shared_from_this();

        BOOST_TEST_EQ( p, q );
        BOOST_TEST( !( p < q ) && !( q < p ) );
    }
    catch( mars_boost::bad_weak_ptr const & )
    {
        BOOST_ERROR( "p->shared_from_this() failed" );
    }

    Z v2( *p );

    try
    {
        mars_boost::shared_ptr<Z> q = v2.shared_from_this();
        BOOST_ERROR( "v2.shared_from_this() failed to throw" );
    }
    catch( mars_boost::bad_weak_ptr const & )
    {
    }

    *p = Z();

    try
    {
        mars_boost::shared_ptr<Z> q = p->shared_from_this();

        BOOST_TEST_EQ( p, q );
        BOOST_TEST( !( p < q ) && !( q < p ) );
    }
    catch( mars_boost::bad_weak_ptr const & )
    {
        BOOST_ERROR( "p->shared_from_this() threw bad_weak_ptr after *p = Z()" );
    }

    {
        mars_boost::shared_ptr<Z> p2( p.get(), null_deleter );
    }

    try
    {
        mars_boost::shared_ptr<Z> q = p->shared_from_this();

        BOOST_TEST_EQ( p, q );
        BOOST_TEST( !( p < q ) && !( q < p ) );
    }
    catch( mars_boost::bad_weak_ptr const& )
    {
        BOOST_ERROR( "p->shared_from_this() failed" );
    }

    return mars_boost::report_errors();
}
