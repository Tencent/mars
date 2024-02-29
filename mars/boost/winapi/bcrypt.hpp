/*
 * Copyright 2017 James E. King, III
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef BOOST_WINAPI_BCRYPT_HPP_INCLUDED_
#define BOOST_WINAPI_BCRYPT_HPP_INCLUDED_

#include <boost/winapi/basic_types.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if BOOST_USE_WINAPI_VERSION >= BOOST_WINAPI_VERSION_WIN6

#if BOOST_WINAPI_PARTITION_APP_SYSTEM

#if defined(BOOST_USE_WINDOWS_H)
#include <bcrypt.h>
#endif

#include <boost/winapi/detail/header.hpp>

#if defined(BOOST_USE_WINDOWS_H)

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace winapi {
typedef ::BCRYPT_ALG_HANDLE BCRYPT_ALG_HANDLE_;
}}

#else // defined(BOOST_USE_WINDOWS_H)

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace winapi {
typedef PVOID_ BCRYPT_ALG_HANDLE_;
}}

extern "C" {

mars_boost::winapi::NTSTATUS_ BOOST_WINAPI_WINAPI_CC
BCryptCloseAlgorithmProvider(
    mars_boost::winapi::BCRYPT_ALG_HANDLE_ hAlgorithm,
    mars_boost::winapi::ULONG_             dwFlags
);

mars_boost::winapi::NTSTATUS_ BOOST_WINAPI_WINAPI_CC
BCryptGenRandom(
    mars_boost::winapi::BCRYPT_ALG_HANDLE_ hAlgorithm,
    mars_boost::winapi::PUCHAR_            pbBuffer,
    mars_boost::winapi::ULONG_             cbBuffer,
    mars_boost::winapi::ULONG_             dwFlags
);

mars_boost::winapi::NTSTATUS_ BOOST_WINAPI_WINAPI_CC
BCryptOpenAlgorithmProvider(
    mars_boost::winapi::BCRYPT_ALG_HANDLE_ *phAlgorithm,
    mars_boost::winapi::LPCWSTR_           pszAlgId,
    mars_boost::winapi::LPCWSTR_           pszImplementation,
    mars_boost::winapi::DWORD_             dwFlags
);

} // extern "C"

#endif // defined(BOOST_USE_WINDOWS_H)

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace winapi {

#if defined(BOOST_USE_WINDOWS_H)
const WCHAR_ BCRYPT_RNG_ALGORITHM_[] = BCRYPT_RNG_ALGORITHM;
#else
const WCHAR_ BCRYPT_RNG_ALGORITHM_[] = L"RNG";
#endif

using ::BCryptCloseAlgorithmProvider;
using ::BCryptGenRandom;
using ::BCryptOpenAlgorithmProvider;

} // winapi
} // boost

#include <boost/winapi/detail/footer.hpp>

#endif // BOOST_WINAPI_PARTITION_APP_SYSTEM

#endif // BOOST_USE_WINAPI_VERSION >= BOOST_WINAPI_VERSION_WIN6

#endif // BOOST_WINAPI_BCRYPT_HPP_INCLUDED_
