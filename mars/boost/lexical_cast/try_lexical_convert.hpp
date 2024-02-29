// Copyright Kevlin Henney, 2000-2005.
// Copyright Alexander Nasonov, 2006-2010.
// Copyright Antony Polukhin, 2011-2023.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// what:  lexical_cast custom keyword cast
// who:   contributed by Kevlin Henney,
//        enhanced with contributions from Terje Slettebo,
//        with additional fixes and suggestions from Gennaro Prota,
//        Beman Dawes, Dave Abrahams, Daryle Walker, Peter Dimov,
//        Alexander Nasonov, Antony Polukhin, Justin Viiret, Michael Hofmann,
//        Cheng Yang, Matthew Bradbury, David W. Birdsall, Pavel Korzh and other Boosters
// when:  November 2000, March 2003, June 2005, June 2006, March 2011 - 2014

#ifndef BOOST_LEXICAL_CAST_TRY_LEXICAL_CONVERT_HPP
#define BOOST_LEXICAL_CAST_TRY_LEXICAL_CONVERT_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#if defined(__clang__) || (defined(__GNUC__) && \
    !(defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)) && \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif


#include <boost/type_traits/conditional.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/type_identity.hpp>
#include <string>

#include <boost/lexical_cast/detail/buffer_view.hpp>
#include <boost/lexical_cast/detail/converter_lexical.hpp>
#include <boost/lexical_cast/detail/converter_numeric.hpp>
#include <boost/lexical_cast/detail/is_character.hpp>

#include <boost/container/container_fwd.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
    namespace detail
    {
        template<typename T>
        struct is_stdstring
            : mars_boost::false_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_stdstring< std::basic_string<CharT, Traits, Alloc> >
            : mars_boost::true_type
        {};

        // Sun Studio has problem with partial specialization of templates differing only in namespace.
        // We workaround that by making `is_booststring` trait, instead of specializing `is_stdstring` for `mars_boost::container::basic_string`.
        template<typename T>
        struct is_booststring
            : mars_boost::false_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_booststring< mars_boost::container::basic_string<CharT, Traits, Alloc> >
            : mars_boost::true_type
        {};

        template<typename Target, typename Source>
        struct is_arithmetic_and_not_xchars
        {
            typedef mars_boost::integral_constant<
                bool,
                !(mars_boost::detail::is_character<Target>::value) &&
                    !(mars_boost::detail::is_character<Source>::value) &&
                    mars_boost::is_arithmetic<Source>::value &&
                    mars_boost::is_arithmetic<Target>::value
                > type;

            BOOST_STATIC_CONSTANT(bool, value = (
                type::value
            ));
        };

        /*
         * is_xchar_to_xchar<Target, Source>::value is true,
         * Target and Souce are char types of the same size 1 (char, signed char, unsigned char).
         */
        template<typename Target, typename Source>
        struct is_xchar_to_xchar
        {
            typedef mars_boost::integral_constant<
                bool,
                sizeof(Source) == sizeof(Target) &&
                     sizeof(Source) == sizeof(char) &&
                     mars_boost::detail::is_character<Target>::value &&
                     mars_boost::detail::is_character<Source>::value
                > type;

            BOOST_STATIC_CONSTANT(bool, value = (
                type::value
            ));
        };

        template<typename Target, typename Source>
        struct is_char_array_to_stdstring
            : mars_boost::false_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_char_array_to_stdstring< std::basic_string<CharT, Traits, Alloc>, CharT* >
            : mars_boost::true_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_char_array_to_stdstring< std::basic_string<CharT, Traits, Alloc>, const CharT* >
            : mars_boost::true_type
        {};

        // Sun Studio has problem with partial specialization of templates differing only in namespace.
        // We workaround that by making `is_char_array_to_booststring` trait, instead of specializing `is_char_array_to_stdstring` for `mars_boost::container::basic_string`.
        template<typename Target, typename Source>
        struct is_char_array_to_booststring
            : mars_boost::false_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_char_array_to_booststring< mars_boost::container::basic_string<CharT, Traits, Alloc>, CharT* >
            : mars_boost::true_type
        {};

        template<typename CharT, typename Traits, typename Alloc>
        struct is_char_array_to_booststring< mars_boost::container::basic_string<CharT, Traits, Alloc>, const CharT* >
            : mars_boost::true_type
        {};

        template <typename Target, typename Source>
        struct copy_converter_impl
        {
// MSVC fail to forward an array (DevDiv#555157 "SILENT BAD CODEGEN triggered by perfect forwarding",
// fixed in 2013 RTM).
#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && (!defined(BOOST_MSVC) || BOOST_MSVC >= 1800)
            template <class T>
            static inline bool try_convert(T&& arg, Target& result) {
                result = static_cast<T&&>(arg); // eqaul to `result = std::forward<T>(arg);`
                return true;
            }
#else
            static inline bool try_convert(const Source& arg, Target& result) {
                result = arg;
                return true;
            }
#endif
        };
    }

    namespace conversion { namespace detail {

        template <typename Target, typename Source>
        inline bool try_lexical_convert(const Source& arg, Target& result)
        {
            typedef typename mars_boost::detail::array_to_pointer_decay<Source>::type src;

            typedef mars_boost::integral_constant<
                bool,
                mars_boost::detail::is_xchar_to_xchar<Target, src >::value ||
                mars_boost::detail::is_char_array_to_stdstring<Target, src >::value ||
                mars_boost::detail::is_char_array_to_booststring<Target, src >::value ||
                (
                     mars_boost::is_same<Target, src >::value &&
                     (mars_boost::detail::is_stdstring<Target >::value || mars_boost::detail::is_booststring<Target >::value)
                ) ||
                (
                     mars_boost::is_same<Target, src >::value &&
                     mars_boost::detail::is_character<Target >::value
                )
            > shall_we_copy_t;

            typedef mars_boost::detail::is_arithmetic_and_not_xchars<Target, src >
                shall_we_copy_with_dynamic_check_t;

            // We do evaluate second `if_` lazily to avoid unnecessary instantiations
            // of `shall_we_copy_with_dynamic_check_t` and improve compilation times.
            typedef typename mars_boost::conditional<
                shall_we_copy_t::value,
                mars_boost::type_identity<mars_boost::detail::copy_converter_impl<Target, src > >,
                mars_boost::conditional<
                     shall_we_copy_with_dynamic_check_t::value,
                     mars_boost::detail::dynamic_num_converter_impl<Target, src >,
                     mars_boost::detail::lexical_converter_impl<Target, src >
                >
            >::type caster_type_lazy;

            typedef typename caster_type_lazy::type caster_type;

            return caster_type::try_convert(arg, result);
        }

        template <typename Target, typename CharacterT>
        inline bool try_lexical_convert(const CharacterT* chars, std::size_t count, Target& result)
        {
            static_assert(
                mars_boost::detail::is_character<CharacterT>::value,
                "This overload of try_lexical_convert is meant to be used only with arrays of characters."
            );
            return ::mars_boost::conversion::detail::try_lexical_convert(
                ::mars_boost::conversion::detail::make_buffer_view(chars, chars + count),
                result
            );
        }

    }} // namespace conversion::detail

    namespace conversion {
        // ADL barrier
        using ::mars_boost::conversion::detail::try_lexical_convert;
    }

} // namespace mars_boost

#if defined(__clang__) || (defined(__GNUC__) && \
    !(defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)) && \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
#pragma GCC diagnostic pop
#endif

#endif // BOOST_LEXICAL_CAST_TRY_LEXICAL_CONVERT_HPP

