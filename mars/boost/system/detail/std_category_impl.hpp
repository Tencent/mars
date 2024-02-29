#ifndef BOOST_SYSTEM_DETAIL_STD_CATEGORY_IMPL_HPP_INCLUDED
#define BOOST_SYSTEM_DETAIL_STD_CATEGORY_IMPL_HPP_INCLUDED

// Support for interoperability between Boost.System and <system_error>
//
// Copyright 2018, 2021 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See library home page at http://www.boost.org/libs/system

#include <boost/system/detail/std_category.hpp>
#include <boost/system/detail/error_condition.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/system/detail/generic_category.hpp>

//

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{

namespace system
{

namespace detail
{

inline bool std_category::equivalent( int code, const std::error_condition & condition ) const BOOST_NOEXCEPT
{
    if( condition.category() == *this )
    {
        mars_boost::system::error_condition bn( condition.value(), *pc_ );
        return pc_->equivalent( code, bn );
    }
    else if( condition.category() == std::generic_category() || condition.category() == mars_boost::system::generic_category() )
    {
        mars_boost::system::error_condition bn( condition.value(), mars_boost::system::generic_category() );
        return pc_->equivalent( code, bn );
    }

#ifndef BOOST_NO_RTTI

    else if( std_category const* pc2 = dynamic_cast< std_category const* >( &condition.category() ) )
    {
        mars_boost::system::error_condition bn( condition.value(), *pc2->pc_ );
        return pc_->equivalent( code, bn );
    }

#endif

    else
    {
        return default_error_condition( code ) == condition;
    }
}

inline bool std_category::equivalent( const std::error_code & code, int condition ) const BOOST_NOEXCEPT
{
    if( code.category() == *this )
    {
        mars_boost::system::error_code bc( code.value(), *pc_ );
        return pc_->equivalent( bc, condition );
    }
    else if( code.category() == std::generic_category() || code.category() == mars_boost::system::generic_category() )
    {
        mars_boost::system::error_code bc( code.value(), mars_boost::system::generic_category() );
        return pc_->equivalent( bc, condition );
    }

#ifndef BOOST_NO_RTTI

    else if( std_category const* pc2 = dynamic_cast< std_category const* >( &code.category() ) )
    {
        mars_boost::system::error_code bc( code.value(), *pc2->pc_ );
        return pc_->equivalent( bc, condition );
    }

#endif

    else if( *pc_ == mars_boost::system::generic_category() )
    {
        return std::generic_category().equivalent( code, condition );
    }
    else
    {
        return false;
    }
}

} // namespace detail

} // namespace system

} // namespace mars_boost

#endif // #ifndef BOOST_SYSTEM_DETAIL_STD_CATEGORY_IMPL_HPP_INCLUDED
