//  boost thread_clock.cpp  -----------------------------------------------------------//

//  Copyright Beman Dawes 1994, 2006, 2008
//  Copyright Vicente J. Botet Escriba 2009-2011
//  Copyright Christopher Brown 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/chrono for documentation.

//--------------------------------------------------------------------------------------//

#include <boost/assert.hpp>
#include <boost/chrono/config.hpp>
#include <boost/chrono/thread_clock.hpp>
#include <cassert>

# include <mach/thread_act.h>
# include <pthread.h>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace chrono {

    thread_clock::time_point thread_clock::now( ) BOOST_NOEXCEPT
    {
        // get the thread port (borrowing pthread's reference)
        mach_port_t port = pthread_mach_thread_np(pthread_self());

        // get the thread info
        thread_basic_info_data_t info;
        mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
        if ( thread_info(port, THREAD_BASIC_INFO, (thread_info_t)&info, &count) != KERN_SUCCESS )
        {
            BOOST_ASSERT(0 && "Boost::Chrono - Internal Error");
            return time_point();
        }

        // convert to nanoseconds
        duration user = duration(
                static_cast<thread_clock::rep>( info.user_time.seconds ) * 1000000000
                        + static_cast<thread_clock::rep>(info.user_time.microseconds ) * 1000);

        duration system = duration(
                static_cast<thread_clock::rep>( info.system_time.seconds ) * 1000000000
                        + static_cast<thread_clock::rep>( info.system_time.microseconds ) * 1000);

        return time_point( user + system );
    }

#if !defined BOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING
    thread_clock::time_point thread_clock::now( system::error_code & ec )
    {
        // get the thread port (borrowing pthread's reference)
        mach_port_t port = pthread_mach_thread_np(pthread_self());

        // get the thread info
        thread_basic_info_data_t info;
        mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
        if ( thread_info(port, THREAD_BASIC_INFO, (thread_info_t)&info, &count) != KERN_SUCCESS )
        {
            if (::mars_boost::chrono::is_throws(ec))
            {
                mars_boost::throw_exception(
                        system::system_error(
                                EINVAL,
                                ::mars_boost::system::system_category(),
                                "chrono::thread_clock" ));
            }
            else
            {
                ec.assign( errno, ::mars_boost::system::system_category() );
                return time_point();
            }
        }
        if (!::mars_boost::chrono::is_throws(ec))
        {
            ec.clear();
        }

        // convert to nanoseconds
        duration user = duration(
                static_cast<thread_clock::rep>( info.user_time.seconds ) * 1000000000
                        + static_cast<thread_clock::rep>(info.user_time.microseconds ) * 1000);

        duration system = duration(
                static_cast<thread_clock::rep>( info.system_time.seconds ) * 1000000000
                        + static_cast<thread_clock::rep>( info.system_time.microseconds ) * 1000);

        return time_point( user + system );
    }
#endif
} }
