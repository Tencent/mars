//
//  typelist.h
//  ngowl
//
//  Created by peterfan on 2018/12/14.
//  Copyright © 2018年 peterfan. All rights reserved.
//

#ifndef _MPL_TYPE_LIST_H_
#define _MPL_TYPE_LIST_H_

#include <cstddef>
#include <type_traits>

namespace owl {

struct null_t {};
    
template <typename H, typename T>
struct typelist {
    using Head = H;
    using Tail = T;
};
    
// make_typelist
template <typename... Types> struct make_typelist_;

template <typename... Types>
using make_typelist = typename make_typelist_<Types...>::type;

template <>
struct make_typelist_<> {
    using type = null_t;
};

template <typename Head, typename... Types>
struct make_typelist_<Head, Types...> {
    using type = typelist<Head, typename make_typelist_<Types...>::type>;
};

// typelist_size
template <typename Typelist> struct typelist_size;

template <>
struct typelist_size<null_t> {
    enum { value = 0 };
};

template <typename Head, typename Tail>
struct typelist_size<typelist<Head, Tail>> {
    enum { value = typelist_size<Tail>::value + 1 };
};

// typelist_find
template <typename Typelist, typename T> struct typelist_find;

template <typename T>
struct typelist_find<null_t, T> {
    enum { value = -1 };
};

template <typename T, typename Tail>
struct typelist_find<typelist<T, Tail>, T> {
    enum { value = 0 };
};

template <typename Head, typename Tail, typename T>
struct typelist_find<typelist<Head, Tail>, T> {
    using type = typelist_find<Tail, T>;
    enum { value = type::value == -1 ? -1 : type::value + 1 };
};

// typelist_at
template <typename Typelist, size_t I> struct typelist_at_;

template <typename Typelist, size_t I>
using typelist_at = typename typelist_at_<Typelist, I>::type;

template <size_t I>
struct typelist_at_<null_t, I> {
    using type = null_t;
};

template <typename T, typename Tail>
struct typelist_at_<typelist<T, Tail>, 0> {
    using type = T;
};

template <typename Head, typename Tail, size_t I>
struct typelist_at_<typelist<Head, Tail>, I> {
    using type = typename typelist_at_<Tail, I - 1>::type;
};

// typelist_contains
template <typename Typelist, typename T> struct typelist_contains {
    enum { value = typelist_find<Typelist, T>::value != -1 };
};


template <typename T, typename U> struct pair {
    using first = T;
    using second = U;
};

// make_typemap
template <typename... Types>
using make_typemap = make_typelist<Types...>;

// typemap_get
template <typename Typemap, typename Key> struct typemap_get_;

template <typename Typemap, typename Key>
using typemap_get = typename typemap_get_<Typemap, Key>::type;

template <typename Key>
struct typemap_get_<null_t, Key> {
    using type = null_t;
};

template <typename Head, typename Tail, typename Key>
struct typemap_get_<typelist<Head, Tail>, Key> {
    using type = typename typemap_get_<Tail, Key>::type;
};

template <typename Value, typename Tail, typename Key>
struct typemap_get_<typelist<pair<Key, Value>, Tail>, Key> {
    using type = Value;
};

// typemap_contains
template <typename Typemap, typename Key> struct typemap_contains {
    enum { value = !std::is_same<typemap_get<Typemap, Key>, null_t>::value };
};

template <typename Typemap, bool IsKeyList> struct typemap_as_list_;

template <bool IsKeyList>
struct typemap_as_list_<null_t, IsKeyList> {
    using type = null_t;
};

template <typename Key, typename Value, typename Tail, bool IsKeyList>
struct typemap_as_list_<typelist<pair<Key, Value>, Tail>, IsKeyList> {
    using type = typelist<typename std::conditional<IsKeyList, Key, Value>::type, typename typemap_as_list_<Tail, IsKeyList>::type>;
};

// typemap_key_list
template <typename Typemap>
using typemap_key_list = typename typemap_as_list_<Typemap, true>::type;

// typemap_value_list
template <typename Typemap>
using typemap_value_list = typename typemap_as_list_<Typemap, false>::type;

// typemap_find
template <typename Typemap, typename Key>
using typemap_find = typelist_find<typemap_key_list<Typemap>, Key>;

// typemap_flip
template <typename Typemap> struct typemap_flip_;

template <typename Typemap>
using typemap_flip = typename typemap_flip_<Typemap>::type;

template <>
struct typemap_flip_<null_t> {
    using type = null_t;
};

template <typename Key, typename Value, typename Tail>
struct typemap_flip_<typelist<pair<Key, Value>, Tail>> {
    using type = typelist<pair<Value, Key>, typename typemap_flip_<Tail>::type>;
};

// typemap_flip_get
template <typename Typemap, typename Key>
using typemap_flip_get = typemap_get<typemap_flip<Typemap>, Key>;

// typemap_flip_contains
template <typename Typemap, typename Key>
using typemap_flip_contains = typemap_contains<typemap_flip<Typemap>, Key>;

// typemap_flip_find
template <typename Typemap, typename Key>
using typemap_flip_find = typemap_find<typemap_flip<Typemap>, Key>;

} //namespace owl

#endif /* _MPL_TYPE_LIST_H_ */
