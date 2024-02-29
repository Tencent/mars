// Copyright 2018 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
//
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/config/pragma_message.hpp>

#if defined(BOOST_DISABLE_THREADS)
BOOST_PRAGMA_MESSAGE( "BOOST_DISABLE_THREADS is defined" )
#else
BOOST_PRAGMA_MESSAGE( "BOOST_DISABLE_THREADS is not defined" )
#endif

#if defined(BOOST_NO_CXX11_HDR_ATOMIC)
BOOST_PRAGMA_MESSAGE( "BOOST_NO_CXX11_HDR_ATOMIC is defined" )
#else
BOOST_PRAGMA_MESSAGE( "BOOST_NO_CXX11_HDR_ATOMIC is not defined" )
#endif

void abi_test_1( mars_boost::shared_ptr<void> & p );
mars_boost::shared_ptr<void> abi_test_2( mars_boost::shared_ptr<void> const & p );
mars_boost::shared_ptr<void> abi_test_3();

static int deleter_called;

void deleter( void* )
{
    ++deleter_called;
}

int main()
{
    {
        deleter_called = 0;

        mars_boost::shared_ptr<void> p( static_cast<void*>( 0 ), deleter );

        BOOST_TEST_EQ( p.use_count(), 1 );

        abi_test_1( p );

        BOOST_TEST_EQ( p.use_count(), 0 );
        BOOST_TEST_EQ( deleter_called, 1 );
    }

    {
        deleter_called = 0;

        mars_boost::shared_ptr<void> p1( static_cast<void*>( 0 ), deleter );

        BOOST_TEST_EQ( p1.use_count(), 1 );

        mars_boost::shared_ptr<void> p2 = abi_test_2( p1 );

        BOOST_TEST_EQ( p1.use_count(), 2 );
        BOOST_TEST_EQ( p2.use_count(), 2 );

        p1.reset();

        BOOST_TEST_EQ( p2.use_count(), 1 );

        p2.reset();

        BOOST_TEST_EQ( deleter_called, 1 );
    }

    {
        mars_boost::shared_ptr<void> p = abi_test_3();

        BOOST_TEST_EQ( p.use_count(), 1 );

        p.reset();

        BOOST_TEST_EQ( p.use_count(), 0 );
    }

    return mars_boost::report_errors();
}
