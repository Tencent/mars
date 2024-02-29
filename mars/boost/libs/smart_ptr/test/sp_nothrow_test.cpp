//
// sp_nothrow_test.cpp
//
// Copyright 2016 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/config.hpp>

#if defined( BOOST_NO_CXX11_HDR_TYPE_TRAITS )

int main()
{
}

#else

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/core/lightweight_test_trait.hpp>
#include <type_traits>

template<class T> void test_copy()
{
    BOOST_TEST_TRAIT_TRUE(( std::is_nothrow_copy_constructible<T> ));
    BOOST_TEST_TRAIT_TRUE(( std::is_nothrow_copy_assignable<T> ));
}

template<class T> void test_move()
{
    BOOST_TEST_TRAIT_TRUE(( std::is_nothrow_move_constructible<T> ));
    BOOST_TEST_TRAIT_TRUE(( std::is_nothrow_move_assignable<T> ));
}

template<class T> void test_default()
{
    BOOST_TEST_TRAIT_TRUE(( std::is_nothrow_default_constructible<T> ));
}

template<class T> void test_destroy()
{
    BOOST_TEST_TRAIT_TRUE(( std::is_nothrow_destructible<T> ));
}

template<class T> void test_cmd()
{
    test_copy<T>();
    test_move<T>();
    test_default<T>();
}

struct X
{
};

struct Y: public mars_boost::enable_shared_from_this<Y>
{
};

int main()
{
    test_cmd< mars_boost::shared_ptr<X> >();
    test_cmd< mars_boost::shared_array<X> >();
    test_cmd< mars_boost::weak_ptr<X> >();

    test_copy< Y >();
    test_default< Y >();
    test_destroy< Y >();

    // test_move< Y >();
    BOOST_TEST_TRAIT_TRUE(( std::is_nothrow_move_constructible<Y> ));

#if !( defined( BOOST_MSVC ) && BOOST_MSVC == 1700 )

    BOOST_TEST_TRAIT_TRUE(( std::is_nothrow_move_assignable<Y> ));

#endif

    test_default< mars_boost::scoped_ptr<X> >();
    test_default< mars_boost::scoped_array<X> >();

    test_move< mars_boost::intrusive_ptr<X> >();
    test_default< mars_boost::intrusive_ptr<X> >();

    return mars_boost::report_errors();
}

#endif // #if defined( BOOST_NO_CXX11_HDR_TYPE_TRAITS )
