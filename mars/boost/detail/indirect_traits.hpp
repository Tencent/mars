// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef INDIRECT_TRAITS_DWA2002131_HPP
# define INDIRECT_TRAITS_DWA2002131_HPP
# include <boost/type_traits/integral_constant.hpp>
# include <boost/type_traits/is_class.hpp>
# include <boost/type_traits/is_const.hpp>
# include <boost/type_traits/is_function.hpp>
# include <boost/type_traits/is_member_function_pointer.hpp>
# include <boost/type_traits/is_member_pointer.hpp>
# include <boost/type_traits/is_pointer.hpp>
# include <boost/type_traits/is_reference.hpp>
# include <boost/type_traits/is_volatile.hpp>
# include <boost/type_traits/remove_cv.hpp>
# include <boost/type_traits/remove_pointer.hpp>
# include <boost/type_traits/remove_reference.hpp>

# include <boost/detail/select_type.hpp>
# include <boost/detail/workaround.hpp>


namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace detail {

namespace indirect_traits {

template <class T>
struct is_reference_to_const : mars_boost::false_type
{
};

template <class T>
struct is_reference_to_const<T const&> : mars_boost::true_type
{
};

#   if defined(BOOST_MSVC) && _MSC_FULL_VER <= 13102140 // vc7.01 alpha workaround
template<class T>
struct is_reference_to_const<T const volatile&> : mars_boost::true_type
{
};
#   endif

template <class T>
struct is_reference_to_function : mars_boost::false_type
{
};

template <class T>
struct is_reference_to_function<T&> : is_function<T>
{
};

template <class T>
struct is_pointer_to_function : mars_boost::false_type
{
};

// There's no such thing as a pointer-to-cv-function, so we don't need
// specializations for those
template <class T>
struct is_pointer_to_function<T*> : is_function<T>
{
};

template <class T>
struct is_reference_to_member_function_pointer_impl : mars_boost::false_type
{
};

template <class T>
struct is_reference_to_member_function_pointer_impl<T&>
    : is_member_function_pointer<typename remove_cv<T>::type>
{
};


template <class T>
struct is_reference_to_member_function_pointer
    : is_reference_to_member_function_pointer_impl<T>
{
};

template <class T>
struct is_reference_to_function_pointer_aux
    : mars_boost::integral_constant<bool,
          is_reference<T>::value &&
          is_pointer_to_function<
              typename remove_cv<
                  typename remove_reference<T>::type
              >::type
          >::value
      >
{
    // There's no such thing as a pointer-to-cv-function, so we don't need specializations for those
};

template <class T>
struct is_reference_to_function_pointer
    : mars_boost::detail::if_true<
          is_reference_to_function<T>::value
      >::template then<
          mars_boost::false_type
        , is_reference_to_function_pointer_aux<T>
      >::type
{
};

template <class T>
struct is_reference_to_non_const
    : mars_boost::integral_constant<bool,
          is_reference<T>::value &&
          !is_reference_to_const<T>::value
      >
{
};

template <class T>
struct is_reference_to_volatile : mars_boost::false_type
{
};

template <class T>
struct is_reference_to_volatile<T volatile&> : mars_boost::true_type
{
};

#   if defined(BOOST_MSVC) && _MSC_FULL_VER <= 13102140 // vc7.01 alpha workaround
template <class T>
struct is_reference_to_volatile<T const volatile&> : mars_boost::true_type
{
};
#   endif


template <class T>
struct is_reference_to_pointer : mars_boost::false_type
{
};

template <class T>
struct is_reference_to_pointer<T*&> : mars_boost::true_type
{
};

template <class T>
struct is_reference_to_pointer<T* const&> : mars_boost::true_type
{
};

template <class T>
struct is_reference_to_pointer<T* volatile&> : mars_boost::true_type
{
};

template <class T>
struct is_reference_to_pointer<T* const volatile&> : mars_boost::true_type
{
};

template <class T>
struct is_reference_to_class
    : mars_boost::integral_constant<bool,
          is_reference<T>::value &&
          is_class<
              typename remove_cv<
                  typename remove_reference<T>::type
              >::type
          >::value
      >
{
};

template <class T>
struct is_pointer_to_class
    : mars_boost::integral_constant<bool,
          is_pointer<T>::value &&
          is_class<
              typename remove_cv<
                  typename remove_pointer<T>::type
              >::type
          >::value
      >
{
};


}

using namespace indirect_traits;

}} // namespace mars_boost::python::detail

#endif // INDIRECT_TRAITS_DWA2002131_HPP
