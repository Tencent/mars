// Copyright 2011, 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/config.hpp>

#if defined(BOOST_NO_CXX11_HDR_UNORDERED_SET)

int main() {}

#else

#include <unordered_set>

int main()
{
    std::unordered_set< mars_boost::weak_ptr<void> > set;

    mars_boost::shared_ptr<int> p1( (int*)0 );
    mars_boost::shared_ptr<int> p2( p1 );
    mars_boost::shared_ptr<void> p3( p1 );

    set.insert( p1 );
    set.insert( p2 );
    set.insert( p3 );

    BOOST_TEST_EQ( set.size(), 1 );

    mars_boost::weak_ptr<int> q1( p1 );
    mars_boost::weak_ptr<int> q2( p2 );
    mars_boost::weak_ptr<void> q3( p3 );
    mars_boost::weak_ptr<int> q4( q2 );
    mars_boost::weak_ptr<void> q5( q3 );

    set.insert( q1 );
    set.insert( q2 );
    set.insert( q3 );
    set.insert( q4 );
    set.insert( q5 );

    BOOST_TEST_EQ( set.size(), 1 );

    mars_boost::shared_ptr<int> p6( (int*)0 );

    set.insert( p6 );

    BOOST_TEST_EQ( set.size(), 2 );

    mars_boost::weak_ptr<int> q6( p6 );

    set.insert( q6 );

    BOOST_TEST_EQ( set.size(), 2 );

    BOOST_TEST_EQ( set.count( q1 ), 1 );
    BOOST_TEST_EQ( set.count( q2 ), 1 );
    BOOST_TEST_EQ( set.count( q3 ), 1 );
    BOOST_TEST_EQ( set.count( q4 ), 1 );
    BOOST_TEST_EQ( set.count( q5 ), 1 );
    BOOST_TEST_EQ( set.count( q6 ), 1 );

    mars_boost::shared_ptr<int> p7( (int*)0 );
    mars_boost::weak_ptr<int> q7( p7 );

    BOOST_TEST_EQ( set.count( q7 ), 0 );

    p1.reset();
    p2.reset();
    p3.reset();
    p6.reset();
    p7.reset();

    BOOST_TEST_EQ( set.count( q1 ), 1 );
    BOOST_TEST_EQ( set.count( q2 ), 1 );
    BOOST_TEST_EQ( set.count( q3 ), 1 );
    BOOST_TEST_EQ( set.count( q4 ), 1 );
    BOOST_TEST_EQ( set.count( q5 ), 1 );
    BOOST_TEST_EQ( set.count( q6 ), 1 );
    BOOST_TEST_EQ( set.count( q7 ), 0 );

    return mars_boost::report_errors();
}

#endif // #if defined(BOOST_NO_CXX11_HDR_UNORDERED_SET)
