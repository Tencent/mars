// Copyright 2011, 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/config.hpp>
#include <functional>

#if defined(BOOST_NO_CXX11_HDR_FUNCTIONAL)

int main() {}

#else

int main()
{
    {
        mars_boost::shared_ptr<int> p1, p2( new int );
        mars_boost::weak_ptr<int> q1( p1 ), q2( p2 ), q3;

        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int> >()( q1 ), q1.owner_hash_value() );
        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int> >()( q2 ), q2.owner_hash_value() );
        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int> >()( q3 ), q3.owner_hash_value() );

        p2.reset();

        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int> >()( q2 ), q2.owner_hash_value() );
    }

    {
        mars_boost::shared_ptr<int[]> p1, p2( new int[1] );
        mars_boost::weak_ptr<int[]> q1( p1 ), q2( p2 ), q3;

        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int[]> >()( q1 ), q1.owner_hash_value() );
        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int[]> >()( q2 ), q2.owner_hash_value() );
        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int[]> >()( q3 ), q3.owner_hash_value() );

        p2.reset();

        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int[]> >()( q2 ), q2.owner_hash_value() );
    }

    {
        mars_boost::shared_ptr<int[1]> p1, p2( new int[1] );
        mars_boost::weak_ptr<int[1]> q1( p1 ), q2( p2 ), q3;

        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int[1]> >()( q1 ), q1.owner_hash_value() );
        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int[1]> >()( q2 ), q2.owner_hash_value() );
        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int[1]> >()( q3 ), q3.owner_hash_value() );

        p2.reset();

        BOOST_TEST_EQ( std::hash< mars_boost::weak_ptr<int[1]> >()( q2 ), q2.owner_hash_value() );
    }

    return mars_boost::report_errors();
}

#endif // #if defined(BOOST_NO_CXX11_HDR_FUNCTIONAL)
