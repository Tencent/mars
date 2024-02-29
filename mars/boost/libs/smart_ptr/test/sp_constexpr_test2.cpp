//
// sp_constexpr_test2.cpp
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
    int v_;

    constexpr X() BOOST_NOEXCEPT: v_( 1 )
    {
    }
};

struct Z
{
    Z();
};

static Z z;
static X x;

Z::Z()
{
    BOOST_TEST_EQ( x.v_, 1 );
}

int main()
{
    return mars_boost::report_errors();
}

#endif // #if defined( BOOST_NO_CXX11_CONSEXPR )
