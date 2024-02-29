#ifndef BOOST_MP11_DETAIL_MP_VOID_HPP_INCLUDED
#define BOOST_MP11_DETAIL_MP_VOID_HPP_INCLUDED

//  Copyright 2015-2017 Peter Dimov.
//
//  Distributed under the Boost Software License, Version 1.0.
//
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
namespace mp11
{

// mp_void<T...>
namespace detail
{

template<class... T> struct mp_void_impl
{
    using type = void;
};

} // namespace detail

template<class... T> using mp_void = typename detail::mp_void_impl<T...>::type;

} // namespace mp11
} // namespace mars_boost

#endif // #ifndef BOOST_MP11_DETAIL_MP_VOID_HPP_INCLUDED
