#ifndef BOOST_WIN32_THREAD_PRIMITIVES_HPP
#define BOOST_WIN32_THREAD_PRIMITIVES_HPP

//  win32_thread_primitives.hpp
//
//  (C) Copyright 2005-7 Anthony Williams
//  (C) Copyright 2007 David Deakins
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/assert.hpp>
#include <boost/detail/interlocked.hpp>
#include <boost/predef/platform.h>
#include <boost/thread/detail/config.hpp>
#include <boost/thread/exceptions.hpp>
#include <boost/throw_exception.hpp>

#include <boost/winapi/access_rights.hpp>
#include <boost/winapi/basic_types.hpp>
#include <boost/winapi/config.hpp>
#include <boost/winapi/event.hpp>
#include <boost/winapi/get_current_process.hpp>
#include <boost/winapi/get_current_process_id.hpp>
#include <boost/winapi/get_current_thread.hpp>
#include <boost/winapi/get_current_thread_id.hpp>
#include <boost/winapi/handles.hpp>
#include <boost/winapi/semaphore.hpp>
#include <boost/winapi/system.hpp>
#include <boost/winapi/thread.hpp>
#include <boost/winapi/wait.hpp>

//#include <boost/winapi/synchronization.hpp>
#include <algorithm>
#include <boost/thread/win32/interlocked_read.hpp>

#if BOOST_PLAT_WINDOWS_RUNTIME
#include <thread>
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
    namespace detail
    {
        namespace win32
        {
            typedef ::mars_boost::winapi::HANDLE_ handle;
            typedef ::mars_boost::winapi::SYSTEM_INFO_ system_info;
            typedef ::mars_boost::winapi::ULONGLONG_ ticks_type;
            unsigned const infinite=::mars_boost::winapi::INFINITE_;
            unsigned const timeout=::mars_boost::winapi::WAIT_TIMEOUT_;
            handle const invalid_handle_value=::mars_boost::winapi::INVALID_HANDLE_VALUE_;
            unsigned const event_modify_state=::mars_boost::winapi::EVENT_MODIFY_STATE_;
            unsigned const synchronize=::mars_boost::winapi::SYNCHRONIZE_;
            unsigned const wait_abandoned=::mars_boost::winapi::WAIT_ABANDONED_;
            unsigned const create_event_initial_set = 0x00000002;
            unsigned const create_event_manual_reset = 0x00000001;
            unsigned const event_all_access = ::mars_boost::winapi::EVENT_ALL_ACCESS_;
            unsigned const semaphore_all_access = mars_boost::winapi::SEMAPHORE_ALL_ACCESS_;
        }
    }
}

#include <boost/config/abi_prefix.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
    namespace detail
    {
        namespace win32
        {
            namespace detail { typedef ticks_type (BOOST_WINAPI_WINAPI_CC *gettickcount64_t)(); }
            extern BOOST_THREAD_DECL mars_boost::detail::win32::detail::gettickcount64_t gettickcount64;

            enum event_type
            {
                auto_reset_event=false,
                manual_reset_event=true
            };

            enum initial_event_state
            {
                event_initially_reset=false,
                event_initially_set=true
            };

            inline handle create_event(
#if !defined(BOOST_NO_ANSI_APIS)
                const char *mutex_name,
#else
                const wchar_t *mutex_name,
#endif
                event_type type,
                initial_event_state state)
            {
#if !defined(BOOST_NO_ANSI_APIS)
                handle const res = ::mars_boost::winapi::CreateEventA(0, type, state, mutex_name);
#elif BOOST_USE_WINAPI_VERSION < BOOST_WINAPI_VERSION_VISTA
                handle const res = ::mars_boost::winapi::CreateEventW(0, type, state, mutex_name);
#else
                handle const res = ::mars_boost::winapi::CreateEventExW(
                    0,
                    mutex_name,
                    (type ? create_event_manual_reset : 0) | (state ? create_event_initial_set : 0),
                    event_all_access);
#endif
                return res;
            }

            inline handle create_anonymous_event(event_type type,initial_event_state state)
            {
                handle const res = create_event(0, type, state);
                if(!res)
                {
                    mars_boost::throw_exception(thread_resource_error());
                }
                return res;
            }

            inline handle create_anonymous_semaphore_nothrow(long initial_count,long max_count)
            {
#if !defined(BOOST_NO_ANSI_APIS)
                handle const res=::mars_boost::winapi::CreateSemaphoreA(0,initial_count,max_count,0);
#else
#if BOOST_USE_WINAPI_VERSION < BOOST_WINAPI_VERSION_VISTA
                handle const res=::mars_boost::winapi::CreateSemaphoreEx(0,initial_count,max_count,0,0);
#else
                handle const res=::mars_boost::winapi::CreateSemaphoreExW(0,initial_count,max_count,0,0,semaphore_all_access);
#endif
#endif
                return res;
            }

            inline handle create_anonymous_semaphore(long initial_count,long max_count)
            {
                handle const res=create_anonymous_semaphore_nothrow(initial_count,max_count);
                if(!res)
                {
                    mars_boost::throw_exception(thread_resource_error());
                }
                return res;
            }

            inline handle duplicate_handle(handle source)
            {
                handle const current_process=::mars_boost::winapi::GetCurrentProcess();
                long const same_access_flag=2;
                handle new_handle=0;
                bool const success=::mars_boost::winapi::DuplicateHandle(current_process,source,current_process,&new_handle,0,false,same_access_flag)!=0;
                if(!success)
                {
                    mars_boost::throw_exception(thread_resource_error());
                }
                return new_handle;
            }

            inline void release_semaphore(handle semaphore,long count)
            {
                BOOST_VERIFY(::mars_boost::winapi::ReleaseSemaphore(semaphore,count,0)!=0);
            }

            inline void get_system_info(system_info *info)
            {
#if BOOST_PLAT_WINDOWS_RUNTIME
                ::mars_boost::winapi::GetNativeSystemInfo(info);
#else
                ::mars_boost::winapi::GetSystemInfo(info);
#endif
            }

            inline void sleep(unsigned long milliseconds)
            {
                if(milliseconds == 0)
                {
#if BOOST_PLAT_WINDOWS_RUNTIME
                    std::this_thread::yield();
#else
                    ::mars_boost::winapi::Sleep(0);
#endif
                }
                else
                {
#if BOOST_PLAT_WINDOWS_RUNTIME
                    ::mars_boost::winapi::WaitForSingleObjectEx(::mars_boost::winapi::GetCurrentThread(), milliseconds, 0);
#else
                    ::mars_boost::winapi::Sleep(milliseconds);
#endif
                }
            }

#if BOOST_PLAT_WINDOWS_RUNTIME
            class BOOST_THREAD_DECL scoped_winrt_thread
            {
            public:
                scoped_winrt_thread() : m_completionHandle(invalid_handle_value)
                {}

                ~scoped_winrt_thread()
                {
                    if (m_completionHandle != ::mars_boost::detail::win32::invalid_handle_value)
                    {
                        ::mars_boost::winapi::CloseHandle(m_completionHandle);
                    }
                }

                typedef unsigned(__stdcall * thread_func)(void *);
                bool start(thread_func address, void *parameter, unsigned int *thrdId);

                handle waitable_handle() const
                {
                    BOOST_ASSERT(m_completionHandle != ::mars_boost::detail::win32::invalid_handle_value);
                    return m_completionHandle;
                }

            private:
                handle m_completionHandle;
            };
#endif
            class BOOST_THREAD_DECL handle_manager
            {
            private:
                handle handle_to_manage;
                handle_manager(handle_manager&);
                handle_manager& operator=(handle_manager&);

                void cleanup()
                {
                    if(handle_to_manage && handle_to_manage!=invalid_handle_value)
                    {
                        BOOST_VERIFY(::mars_boost::winapi::CloseHandle(handle_to_manage));
                    }
                }

            public:
                explicit handle_manager(handle handle_to_manage_):
                    handle_to_manage(handle_to_manage_)
                {}
                handle_manager():
                    handle_to_manage(0)
                {}

                handle_manager& operator=(handle new_handle)
                {
                    cleanup();
                    handle_to_manage=new_handle;
                    return *this;
                }

                operator handle() const
                {
                    return handle_to_manage;
                }

                handle duplicate() const
                {
                    return duplicate_handle(handle_to_manage);
                }

                void swap(handle_manager& other)
                {
                    std::swap(handle_to_manage,other.handle_to_manage);
                }

                handle release()
                {
                    handle const res=handle_to_manage;
                    handle_to_manage=0;
                    return res;
                }

                bool operator!() const
                {
                    return !handle_to_manage;
                }

                ~handle_manager()
                {
                    cleanup();
                }
            };
        }
    }
}

#if defined(BOOST_MSVC) && (_MSC_VER>=1400)  && !defined(UNDER_CE)

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
    namespace detail
    {
        namespace win32
        {
#if _MSC_VER==1400
            extern "C" unsigned char _interlockedbittestandset(long *a,long b);
            extern "C" unsigned char _interlockedbittestandreset(long *a,long b);
#else
            extern "C" unsigned char _interlockedbittestandset(volatile long *a,long b);
            extern "C" unsigned char _interlockedbittestandreset(volatile long *a,long b);
#endif

#pragma intrinsic(_interlockedbittestandset)
#pragma intrinsic(_interlockedbittestandreset)

            inline bool interlocked_bit_test_and_set(long* x,long bit)
            {
                return _interlockedbittestandset(x,bit)!=0;
            }

            inline bool interlocked_bit_test_and_reset(long* x,long bit)
            {
                return _interlockedbittestandreset(x,bit)!=0;
            }

        }
    }
}
#define BOOST_THREAD_BTS_DEFINED
#elif (defined(BOOST_MSVC) || defined(BOOST_INTEL_WIN)) && defined(_M_IX86)
namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
    namespace detail
    {
        namespace win32
        {
            inline bool interlocked_bit_test_and_set(long* x,long bit)
            {
#ifndef BOOST_INTEL_CXX_VERSION
                __asm {
                    mov eax,bit;
                    mov edx,x;
                    lock bts [edx],eax;
                    setc al;
                };
#else
                bool ret;
                __asm {
                    mov eax,bit
                    mov edx,x
                    lock bts [edx],eax
                    setc al
                    mov ret, al
                };
                return ret;

#endif
            }

            inline bool interlocked_bit_test_and_reset(long* x,long bit)
            {
#ifndef BOOST_INTEL_CXX_VERSION
                __asm {
                    mov eax,bit;
                    mov edx,x;
                    lock btr [edx],eax;
                    setc al;
                };
#else
                bool ret;
                __asm {
                    mov eax,bit
                    mov edx,x
                    lock btr [edx],eax
                    setc al
                    mov ret, al
                };
                return ret;

#endif
            }

        }
    }
}
#define BOOST_THREAD_BTS_DEFINED
#endif

#ifndef BOOST_THREAD_BTS_DEFINED

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
    namespace detail
    {
        namespace win32
        {
            inline bool interlocked_bit_test_and_set(long* x,long bit)
            {
                long const value=1<<bit;
                long old=*x;
                do
                {
                    long const current=BOOST_INTERLOCKED_COMPARE_EXCHANGE(x,old|value,old);
                    if(current==old)
                    {
                        break;
                    }
                    old=current;
                }
                while(true) ;
                return (old&value)!=0;
            }

            inline bool interlocked_bit_test_and_reset(long* x,long bit)
            {
                long const value=1<<bit;
                long old=*x;
                do
                {
                    long const current=BOOST_INTERLOCKED_COMPARE_EXCHANGE(x,old&~value,old);
                    if(current==old)
                    {
                        break;
                    }
                    old=current;
                }
                while(true) ;
                return (old&value)!=0;
            }
        }
    }
}
#endif

#include <boost/config/abi_suffix.hpp>

#endif
