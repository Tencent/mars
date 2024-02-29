// Boost.Function library

#ifndef BOOST_FUNCTION_EPILOGUE_HPP
#define BOOST_FUNCTION_EPILOGUE_HPP

// Copyright 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Resolve C++20 issue with fn == bind(...)
// https://github.com/boostorg/function/issues/45

#if !defined(BOOST_FUNCTION_NO_FUNCTION_TYPE_SYNTAX)

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{

namespace _bi
{

template<class R, class F, class L> class bind_t;

} // namespace _bi

template<class S, class R, class F, class L> bool operator==( function<S> const& f, _bi::bind_t<R, F, L> const& b )
{
    return f.contains( b );
}

template<class S, class R, class F, class L> bool operator!=( function<S> const& f, _bi::bind_t<R, F, L> const& b )
{
    return !f.contains( b );
}

} // namespace mars_boost

#endif // #if !defined(BOOST_FUNCTION_NO_FUNCTION_TYPE_SYNTAX)

#endif // #ifndef BOOST_FUNCTION_EPILOGUE_HPP
