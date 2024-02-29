#include <boost/config.hpp>

//  lsp_convertible_test.cpp
//
//  Copyright 2012, 2017 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/type_traits/is_convertible.hpp>

//

class X;

class B
{
};

class D: public B
{
};

using mars_boost::is_convertible;

#define TEST_CV_TRUE_( S1, T, S2, U ) \
    BOOST_TEST(( is_convertible< S1<T>, S2<U> >::value == true )); \
    BOOST_TEST(( is_convertible< S1<T>, S2<const U> >::value == true )); \
    BOOST_TEST(( is_convertible< S1<T>, S2<volatile U> >::value == true )); \
    BOOST_TEST(( is_convertible< S1<T>, S2<const volatile U> >::value == true )); \
    BOOST_TEST(( is_convertible< S1<const T>, S2<U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const T>, S2<const U> >::value == true )); \
    BOOST_TEST(( is_convertible< S1<const T>, S2<volatile U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const T>, S2<const volatile U> >::value == true )); \
    BOOST_TEST(( is_convertible< S1<volatile T>, S2<U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<volatile T>, S2<const U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<volatile T>, S2<volatile U> >::value == true )); \
    BOOST_TEST(( is_convertible< S1<volatile T>, S2<const volatile U> >::value == true )); \
    BOOST_TEST(( is_convertible< S1<const volatile T>, S2<U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const volatile T>, S2<const U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const volatile T>, S2<volatile U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const volatile T>, S2<const volatile U> >::value == true ));

#define TEST_CV_FALSE_( S1, T, S2, U ) \
    BOOST_TEST(( is_convertible< S1<T>, S2<U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<T>, S2<const U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<T>, S2<volatile U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<T>, S2<const volatile U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const T>, S2<U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const T>, S2<const U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const T>, S2<volatile U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const T>, S2<const volatile U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<volatile T>, S2<U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<volatile T>, S2<const U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<volatile T>, S2<volatile U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<volatile T>, S2<const volatile U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const volatile T>, S2<U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const volatile T>, S2<const U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const volatile T>, S2<volatile U> >::value == false )); \
    BOOST_TEST(( is_convertible< S1<const volatile T>, S2<const volatile U> >::value == false ));

using mars_boost::local_shared_ptr;
using mars_boost::shared_ptr;
using mars_boost::weak_ptr;

#define TEST_CV_TRUE( T, U ) \
    TEST_CV_TRUE_( local_shared_ptr, T, shared_ptr, U ) \
    TEST_CV_TRUE_( local_shared_ptr, T, weak_ptr, U )

#define TEST_CV_FALSE( T, U ) \
    TEST_CV_FALSE_( local_shared_ptr, T, shared_ptr, U ) \
    TEST_CV_FALSE_( local_shared_ptr, T, weak_ptr, U )

int main()
{
#if !defined( BOOST_SP_NO_SP_CONVERTIBLE ) && !defined(BOOST_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS)

    TEST_CV_TRUE( X, X )
    TEST_CV_TRUE( X, void )
    TEST_CV_FALSE( void, X )
    TEST_CV_TRUE( D, B )
    TEST_CV_FALSE( B, D )

    TEST_CV_TRUE( X[], X[] )
    TEST_CV_FALSE( D[], B[] )

    TEST_CV_TRUE( X[3], X[3] )
    TEST_CV_FALSE( X[3], X[4] )
    TEST_CV_FALSE( D[3], B[3] )

    TEST_CV_TRUE( X[3], X[] )
    TEST_CV_FALSE( X[], X[3] )

    TEST_CV_TRUE( X[], void )
    TEST_CV_FALSE( void, X[] )

    TEST_CV_TRUE( X[3], void )
    TEST_CV_FALSE( void, X[3] )

#endif

    return mars_boost::report_errors();
}
