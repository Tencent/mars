// Copyright 2011, 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/owner_hash.hpp>
#include <boost/smart_ptr/owner_equal_to.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/config.hpp>

#if defined(BOOST_NO_CXX11_HDR_UNORDERED_SET)

int main() {}

#else

#include <unordered_set>

int main()
{
    std::unordered_set< mars_boost::shared_ptr<void>, mars_boost::owner_hash< mars_boost::shared_ptr<void> >, mars_boost::owner_equal_to< mars_boost::shared_ptr<void> > > set;

    mars_boost::shared_ptr<int> p1( (int*)0 );
    mars_boost::shared_ptr<int> p2( p1 );
    mars_boost::shared_ptr<void> p3( p1 );

    set.insert( p1 );
    set.insert( p2 );
    set.insert( p3 );

    BOOST_TEST_EQ( set.size(), 1 );

    mars_boost::shared_ptr<int> p4( (int*)0 );

    set.insert( p4 );

    BOOST_TEST_EQ( set.size(), 2 );

    BOOST_TEST_EQ( set.count( p1 ), 1 );
    BOOST_TEST_EQ( set.count( p2 ), 1 );
    BOOST_TEST_EQ( set.count( p3 ), 1 );
    BOOST_TEST_EQ( set.count( p4 ), 1 );

    mars_boost::shared_ptr<int> p5( (int*)0 );

    BOOST_TEST_EQ( set.count( p5 ), 0 );

    return mars_boost::report_errors();
}

#endif // #if defined(BOOST_NO_CXX11_HDR_UNORDERED_SET)
