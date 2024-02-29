//
// sp_constexpr_test.cpp
//
// Copyright 2017 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/config.hpp>
#include <boost/config/workaround.hpp>

#define HAVE_CONSTEXPR_INIT

#if defined( BOOST_NO_CXX11_CONSTEXPR )
# undef HAVE_CONSTEXPR_INIT
#endif

#if BOOST_WORKAROUND( BOOST_MSVC, < 1930 )
# undef HAVE_CONSTEXPR_INIT
#endif

#if defined(__clang__) && defined( BOOST_NO_CXX14_CONSTEXPR )
# undef HAVE_CONSTEXPR_INIT
#endif

#if !defined( HAVE_CONSTEXPR_INIT )

int main()
{
}

#else

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/core/lightweight_test.hpp>

struct X: public mars_boost::enable_shared_from_this<X>
{
};

struct Z
{
    Z();
};

static Z z;

static mars_boost::shared_ptr<X> p1;
static mars_boost::weak_ptr<X> p2;

#if !defined( BOOST_NO_CXX11_NULLPTR )
  static mars_boost::shared_ptr<X> p3( nullptr );
#endif

Z::Z()
{
    p1.reset( new X );
    p2 = p1;
#if !defined( BOOST_NO_CXX11_NULLPTR )
    p3.reset( new X );
#endif
}

int main()
{
    BOOST_TEST( p1.get() != 0 );
    BOOST_TEST_EQ( p1.use_count(), 1 );

    BOOST_TEST_EQ( p2.use_count(), 1 );
    BOOST_TEST_EQ( p2.lock(), p1 );

#if !defined( BOOST_NO_CXX11_NULLPTR )

    BOOST_TEST( p3.get() != 0 );
    BOOST_TEST_EQ( p3.use_count(), 1 );

#endif

    return mars_boost::report_errors();
}

#endif // #if defined( BOOST_NO_CXX11_CONSEXPR )
