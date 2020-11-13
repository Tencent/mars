/*
 ============================================================================
 Name		: meta_function.h
 Author		: peterfan
 Version	: 1.0
 Created on	: 2010-10-14
 Copyright	: Copyright (C) 1998 - 2009 TENCENT Inc. All Right Reserved
 Description: 
 ============================================================================
 */

#ifndef _MPL_META_FUNCTION_H_
#define _MPL_META_FUNCTION_H_

#include <cstddef>
#include <type_traits>
#include <tuple>

namespace owl {

// calculate the size of a unary static array
template <typename T, size_t N>
constexpr size_t static_array_size(T (&)[N]) noexcept { return N; }

// check_incomplete_type(T)
#define check_incomplete_type(t) static_assert(sizeof(t) + 1, "check_incomplete_type")

// make_index_seq<1, 2, ..., N>
template <size_t... N>
struct index_seq {};

template <size_t N, size_t... M>
struct make_index_seq: public make_index_seq<N - 1, N - 1, M...> {};

template <size_t... M>
struct make_index_seq<0, M...>: public index_seq<M...> {};

// apply(f, tuple)
template <typename R, typename F, typename Tuple, size_t... I>
R apply_impl(F&& f, Tuple&& t, index_seq<I...>) {
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

template <typename R, typename F, typename Tuple>
R apply(F&& f, Tuple&& t) {
    return apply_impl<R>(std::forward<F>(f), std::forward<Tuple>(t), make_index_seq<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>());
}

// is_iterable<T>
template <typename... Ts>
struct is_iterable_helper {};

template <typename T, typename _ = void>
struct is_iterable_1: public std::integral_constant<bool, false> {};

template<typename T>
struct is_iterable_1<T,
    typename std::conditional<
        false,
        is_iterable_helper<
            decltype(std::declval<T>().begin()),
            decltype(std::declval<T>().end())
        >,
        void
    >::type
>: public std::integral_constant<bool, true> {};


template <typename T, typename _ = void>
struct is_iterable_2: public std::integral_constant<bool, false> {};

template<typename T>
struct is_iterable_2<T,
    typename std::conditional<
        false,
        is_iterable_helper<
            decltype(begin(std::declval<T>())),
            decltype(end(std::declval<T>()))
        >,
        void
    >::type
>: public std::integral_constant<bool, true> {};

template <typename T>
struct is_iterable: std::integral_constant<bool, is_iterable_1<T>::value || is_iterable_2<T>::value> {};

template<typename T, size_t N>
struct is_iterable<T [N]>: public std::integral_constant<bool, !std::is_same<typename std::decay<T>::type, char>::value> {};

// is_char_array<T>
template <typename T>
struct is_char_array: public std::integral_constant<bool, false> {};

template <typename T, size_t N>
struct is_char_array<T [N]>: public std::integral_constant<bool, std::is_same<typename std::decay<T>::type, char>::value> {};

} //namespace owl

#endif /* _MPL_META_FUNCTION_H_ */
