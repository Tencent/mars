//-----------------------------------------------------------------------------
// boost variant/detail/apply_visitor_unary.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002-2003 Eric Friedman
// Copyright (c) 2014-2023 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_VARIANT_DETAIL_APPLY_VISITOR_UNARY_HPP
#define BOOST_VARIANT_DETAIL_APPLY_VISITOR_UNARY_HPP

#include <boost/config.hpp>
#include <utility>

#if !defined(BOOST_NO_CXX14_DECLTYPE_AUTO) && !defined(BOOST_NO_CXX11_DECLTYPE_N3276)
#   include <boost/core/enable_if.hpp>
#   include <boost/mpl/advance.hpp>
#   include <boost/mpl/deref.hpp>
#   include <boost/mpl/distance.hpp>
#   include <boost/mpl/size.hpp>
#   include <boost/type_traits/copy_cv_ref.hpp>
#   include <boost/type_traits/remove_reference.hpp>
#   include <boost/utility/declval.hpp>
#   include <boost/variant/detail/has_result_type.hpp>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

//////////////////////////////////////////////////////////////////////////
// function template apply_visitor(visitor, visitable)
//
// Visits visitable with visitor.
//

//
// nonconst-visitor version:
//

template <typename Visitor, typename Visitable>
inline typename Visitor::result_type
apply_visitor(Visitor& visitor, Visitable&& visitable)
{
    return std::forward<Visitable>(visitable).apply_visitor(visitor);
}

//
// const-visitor version:
//

template <typename Visitor, typename Visitable>
inline typename Visitor::result_type
apply_visitor(const Visitor& visitor, Visitable&& visitable)
{
    return std::forward<Visitable>(visitable).apply_visitor(visitor);
}


#if !defined(BOOST_NO_CXX14_DECLTYPE_AUTO) && !defined(BOOST_NO_CXX11_DECLTYPE_N3276)
#define BOOST_VARIANT_HAS_DECLTYPE_APPLY_VISITOR_RETURN_TYPE

// C++14
namespace detail { namespace variant {

// This class serves only metaprogramming purposes. none of its methods must be called at runtime!
template <class Visitor, class Variant>
struct result_multideduce1 {
    typedef typename remove_reference<Variant>::type::types types;
    typedef typename mars_boost::mpl::begin<types>::type begin_it;
    typedef typename mars_boost::mpl::advance<
        begin_it, mars_boost::mpl::int_<mars_boost::mpl::size<types>::type::value - 1>
    >::type                                         last_it;

    template <class It, class Dummy = void> // avoid explicit specialization in class scope
    struct deduce_impl {
        typedef typename mars_boost::mpl::next<It>::type next_t;
        typedef typename mars_boost::mpl::deref<It>::type value_t;
        typedef decltype(true ? mars_boost::declval< Visitor& >()( mars_boost::declval< copy_cv_ref_t< value_t, Variant > >() )
                              : mars_boost::declval< typename deduce_impl<next_t>::type >()) type;
    };

    template <class Dummy>
    struct deduce_impl<last_it, Dummy> {
        typedef typename mars_boost::mpl::deref<last_it>::type value_t;
        typedef decltype(mars_boost::declval< Visitor& >()( mars_boost::declval< copy_cv_ref_t< value_t, Variant > >() )) type;
    };

    typedef typename deduce_impl<begin_it>::type type;
};

template <class Visitor, class Variant>
struct result_wrapper1
{
    typedef typename result_multideduce1<Visitor, Variant>::type result_type;

    Visitor&& visitor_;
    explicit result_wrapper1(Visitor&& visitor) BOOST_NOEXCEPT
        : visitor_(std::forward<Visitor>(visitor))
    {}

    template <class T>
    result_type operator()(T&& val) const {
        return visitor_(std::forward<T>(val));
    }
};

}} // namespace detail::variant

template <typename Visitor, typename Visitable>
inline decltype(auto) apply_visitor(Visitor&& visitor, Visitable&& visitable,
    typename mars_boost::disable_if<
        mars_boost::detail::variant::has_result_type<Visitor>,
        bool
    >::type = true)
{
    mars_boost::detail::variant::result_wrapper1<Visitor, Visitable> cpp14_vis(std::forward<Visitor>(visitor));
    return std::forward<Visitable>(visitable).apply_visitor(cpp14_vis);
}

#endif // !defined(BOOST_NO_CXX14_DECLTYPE_AUTO) && !defined(BOOST_NO_CXX11_DECLTYPE_N3276)

} // namespace mars_boost

#endif // BOOST_VARIANT_DETAIL_APPLY_VISITOR_UNARY_HPP
