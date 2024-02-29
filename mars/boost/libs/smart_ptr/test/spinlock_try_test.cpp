//
// spinlock_try_test.cpp
//
// Copyright 2008 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/smart_ptr/detail/spinlock.hpp>
#include <boost/core/lightweight_test.hpp>

// Sanity check only

static mars_boost::detail::spinlock sp = BOOST_DETAIL_SPINLOCK_INIT;
static mars_boost::detail::spinlock sp2 = BOOST_DETAIL_SPINLOCK_INIT;

int main()
{
    BOOST_TEST( sp.try_lock() );
    BOOST_TEST( !sp.try_lock() );
    BOOST_TEST( sp2.try_lock() );
    BOOST_TEST( !sp.try_lock() );
    BOOST_TEST( !sp2.try_lock() );
    sp.unlock();
    sp2.unlock();

    sp.lock();
    BOOST_TEST( !sp.try_lock() );
    sp2.lock();
    BOOST_TEST( !sp.try_lock() );
    BOOST_TEST( !sp2.try_lock() );
    sp.unlock();
    sp2.unlock();

    {
        mars_boost::detail::spinlock::scoped_lock lock( sp );
        BOOST_TEST( !sp.try_lock() );
        mars_boost::detail::spinlock::scoped_lock lock2( sp2 );
        BOOST_TEST( !sp.try_lock() );
        BOOST_TEST( !sp2.try_lock() );
    }

    return mars_boost::report_errors();
}
