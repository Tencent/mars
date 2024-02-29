//
// sp_typeinfo_test.cpp
//
// Copyright (c) 2009 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/smart_ptr/detail/sp_typeinfo_.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>

int main()
{
    BOOST_TEST( BOOST_SP_TYPEID_( int ) == BOOST_SP_TYPEID_( int ) );
    BOOST_TEST( BOOST_SP_TYPEID_( int ) != BOOST_SP_TYPEID_( long ) );
    BOOST_TEST( BOOST_SP_TYPEID_( int ) != BOOST_SP_TYPEID_( void ) );

    mars_boost::detail::sp_typeinfo_ const & ti = BOOST_SP_TYPEID_( int );

    mars_boost::detail::sp_typeinfo_ const * pti = &BOOST_SP_TYPEID_( int );
    BOOST_TEST( *pti == ti );

    BOOST_TEST( ti == ti );
    BOOST_TEST( !( ti != ti ) );
    BOOST_TEST( !ti.before( ti ) );

    char const * nti = ti.name();
    std::cout << nti << std::endl;

    mars_boost::detail::sp_typeinfo_ const & tv = BOOST_SP_TYPEID_( void );

    mars_boost::detail::sp_typeinfo_ const * ptv = &BOOST_SP_TYPEID_( void );
    BOOST_TEST( *ptv == tv );

    BOOST_TEST( tv == tv );
    BOOST_TEST( !( tv != tv ) );
    BOOST_TEST( !tv.before( tv ) );

    char const * ntv = tv.name();
    std::cout << ntv << std::endl;

    BOOST_TEST( ti != tv );
    BOOST_TEST( !( ti == tv ) );

    BOOST_TEST( ti.before( tv ) != tv.before( ti ) );

    return mars_boost::report_errors();
}
