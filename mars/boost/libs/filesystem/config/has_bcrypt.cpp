//  Copyright 2020 Andrey Semashev

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See library home page at http://www.boost.org/libs/filesystem

// Include platform_config.hpp first so that windows.h is guaranteed to be not included
#include "platform_config.hpp"

#include <boost/predef/os/windows.h>
#include <boost/predef/os/cygwin.h>
#if !BOOST_OS_WINDOWS && !BOOST_OS_CYGWIN
#error "This config test is for Windows only"
#endif

#include <boost/winapi/config.hpp>
#include <boost/predef/platform.h>
#if !(BOOST_USE_WINAPI_VERSION >= BOOST_WINAPI_VERSION_WIN6 && BOOST_WINAPI_PARTITION_APP_SYSTEM)
#error "No BCrypt API"
#endif

#include <cstddef>
#include <boost/winapi/basic_types.hpp>
#include <boost/winapi/bcrypt.hpp>

int main()
{
    unsigned char buf[16] = {};
    mars_boost::winapi::BCRYPT_ALG_HANDLE_ handle;
    mars_boost::winapi::NTSTATUS_ status = mars_boost::winapi::BCryptOpenAlgorithmProvider(&handle, mars_boost::winapi::BCRYPT_RNG_ALGORITHM_, NULL, 0);
    status = mars_boost::winapi::BCryptGenRandom(handle, reinterpret_cast< mars_boost::winapi::PUCHAR_ >(static_cast< unsigned char* >(buf)), static_cast< mars_boost::winapi::ULONG_ >(sizeof(buf)), 0);
    mars_boost::winapi::BCryptCloseAlgorithmProvider(handle, 0);
}
