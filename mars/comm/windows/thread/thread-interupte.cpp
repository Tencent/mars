// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007 Anthony Williams
// (C) Copyright 2007 David Deakins
// (C) Copyright 2011-2013 Vicente J. Botet Escriba

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x400
#endif

#ifndef WINVER
#define WINVER 0x400
#endif
//#define BOOST_THREAD_VERSION 3

#include <boost/thread/thread_only.hpp>
#include <boost/thread/once.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/detail/tss_hooks.hpp>
#include <boost/thread/future.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#if defined BOOST_THREAD_USES_DATETIME
#include <boost/date_time/posix_time/conversion.hpp>
#endif
#include <boost/thread/csbl/memory/unique_ptr.hpp>
#include <memory>
#include <algorithm>
#ifndef UNDER_CE
#include <process.h>
#endif
#include <stdio.h>
#include <windows.h>
#include <boost/predef/platform.h>

#if BOOST_PLAT_WINDOWS_RUNTIME
#include <mutex>
#include <atomic>
#include <Activation.h>
#include <wrl\client.h>
#include <wrl\event.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\ftm.h>
#include <windows.system.threading.h>
#pragma comment(lib, "runtimeobject.lib")
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
	namespace this_thread
	{
		namespace
		{
			LARGE_INTEGER get_due_time(detail::timeout const&  target_time)
			{
				LARGE_INTEGER due_time = { {0,0} };
				if (target_time.relative)
				{
					detail::win32::ticks_type const elapsed_milliseconds = detail::win32::GetTickCount64_()() - target_time.start;
					LONGLONG const remaining_milliseconds = (target_time.milliseconds - elapsed_milliseconds);
					LONGLONG const hundred_nanoseconds_in_one_millisecond = 10000;

					if (remaining_milliseconds > 0)
					{
						due_time.QuadPart = -(remaining_milliseconds*hundred_nanoseconds_in_one_millisecond);
					}
				}
				else
				{
					SYSTEMTIME target_system_time = { 0,0,0,0,0,0,0,0 };
					target_system_time.wYear = target_time.abs_time.date().year();
					target_system_time.wMonth = target_time.abs_time.date().month();
					target_system_time.wDay = target_time.abs_time.date().day();
					target_system_time.wHour = (WORD)target_time.abs_time.time_of_day().hours();
					target_system_time.wMinute = (WORD)target_time.abs_time.time_of_day().minutes();
					target_system_time.wSecond = (WORD)target_time.abs_time.time_of_day().seconds();

					if (!SystemTimeToFileTime(&target_system_time, ((FILETIME*)&due_time)))
					{
						due_time.QuadPart = 0;
					}
					else
					{
						long const hundred_nanoseconds_in_one_second = 10000000;
						posix_time::time_duration::tick_type const ticks_per_second =
							target_time.abs_time.time_of_day().ticks_per_second();
						if (ticks_per_second > hundred_nanoseconds_in_one_second)
						{
							posix_time::time_duration::tick_type const
								ticks_per_hundred_nanoseconds =
								ticks_per_second / hundred_nanoseconds_in_one_second;
							due_time.QuadPart +=
								target_time.abs_time.time_of_day().fractional_seconds() /
								ticks_per_hundred_nanoseconds;
						}
						else
						{
							due_time.QuadPart +=
								target_time.abs_time.time_of_day().fractional_seconds()*
								(hundred_nanoseconds_in_one_second / ticks_per_second);
						}
					}
				}
				return due_time;
			}
		}

#ifndef UNDER_CE
#if !BOOST_PLAT_WINDOWS_RUNTIME
		namespace detail_
		{
			typedef struct _REASON_CONTEXT {
				ULONG Version;
				DWORD Flags;
				union {
					LPWSTR SimpleReasonString;
					struct {
						HMODULE LocalizedReasonModule;
						ULONG   LocalizedReasonId;
						ULONG   ReasonStringCount;
						LPWSTR  *ReasonStrings;
					} Detailed;
				} Reason;
			} REASON_CONTEXT, *PREASON_CONTEXT;
			//static REASON_CONTEXT default_reason_context={0/*POWER_REQUEST_CONTEXT_VERSION*/, 0x00000001/*POWER_REQUEST_CONTEXT_SIMPLE_STRING*/, (LPWSTR)L"generic"};
			typedef BOOL(WINAPI *setwaitabletimerex_t)(HANDLE, const LARGE_INTEGER *, LONG, PTIMERAPCROUTINE, LPVOID, PREASON_CONTEXT, ULONG);
			static inline BOOL WINAPI SetWaitableTimerEx_emulation(HANDLE hTimer, const LARGE_INTEGER *lpDueTime, LONG lPeriod, PTIMERAPCROUTINE pfnCompletionRoutine, LPVOID lpArgToCompletionRoutine, PREASON_CONTEXT WakeContext, ULONG TolerableDelay)
			{
				return SetWaitableTimer(hTimer, lpDueTime, lPeriod, pfnCompletionRoutine, lpArgToCompletionRoutine, FALSE);
			}
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 6387) // MSVC sanitiser warns that GetModuleHandleA() might fail
#endif
			static inline setwaitabletimerex_t SetWaitableTimerEx()
			{
				static setwaitabletimerex_t setwaitabletimerex_impl;
				if (setwaitabletimerex_impl)
					return setwaitabletimerex_impl;
				void(*addr)() = (void(*)()) GetProcAddress(
#if !defined(BOOST_NO_ANSI_APIS)
					GetModuleHandleA("KERNEL32.DLL"),
#else
					GetModuleHandleW(L"KERNEL32.DLL"),
#endif
					"SetWaitableTimerEx");
				if (addr)
					setwaitabletimerex_impl = (setwaitabletimerex_t)addr;
				else
					setwaitabletimerex_impl = &SetWaitableTimerEx_emulation;
				return setwaitabletimerex_impl;
			}
#ifdef _MSC_VER
#pragma warning(pop)
#endif
		}
#endif
#endif
		bool interruptible_wait(detail::win32::handle handle_to_wait_for, detail::timeout target_time)
		{
			detail::win32::handle handles[4] = { 0 };
			unsigned handle_count = 0;
			unsigned wait_handle_index = ~0U;
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
			unsigned interruption_index = ~0U;
#endif
			unsigned timeout_index = ~0U;
			if (handle_to_wait_for != detail::win32::invalid_handle_value)
			{
				wait_handle_index = handle_count;
				handles[handle_count++] = handle_to_wait_for;
			}
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
			if (detail::get_current_thread_data() && detail::get_current_thread_data()->interruption_enabled)
			{
				interruption_index = handle_count;
				handles[handle_count++] = detail::get_current_thread_data()->interruption_handle;
			}
#endif
			detail::win32::handle_manager timer_handle;

#ifndef UNDER_CE
#if !BOOST_PLAT_WINDOWS_RUNTIME
			// Preferentially use coalescing timers for better power consumption and timer accuracy
			if (!target_time.is_sentinel())
			{
				detail::timeout::remaining_time const time_left = target_time.remaining_milliseconds();
				timer_handle = CreateWaitableTimer(NULL, false, NULL);
				if (timer_handle != 0)
				{
					ULONG tolerable = 32; // Empirical testing shows Windows ignores this when <= 26
					if (time_left.milliseconds / 20 > tolerable)  // 5%
						tolerable = time_left.milliseconds / 20;
					LARGE_INTEGER due_time = get_due_time(target_time);
					//bool const set_time_succeeded=detail_::SetWaitableTimerEx()(timer_handle,&due_time,0,0,0,&detail_::default_reason_context,tolerable)!=0;
					bool const set_time_succeeded = detail_::SetWaitableTimerEx()(timer_handle, &due_time, 0, 0, 0, NULL, tolerable) != 0;
					if (set_time_succeeded)
					{
						timeout_index = handle_count;
						handles[handle_count++] = timer_handle;
					}
				}
			}
#endif
#endif

			bool const using_timer = timeout_index != ~0u;
			detail::timeout::remaining_time time_left(0);

			do
			{
				if (!using_timer)
				{
					time_left = target_time.remaining_milliseconds();
				}

				if (handle_count)
				{
					unsigned long const notified_index = detail::win32::WaitForMultipleObjectsEx(handle_count, handles, false, using_timer ? INFINITE : time_left.milliseconds, 0);
					if (notified_index < handle_count)
					{
						if (notified_index == wait_handle_index)
						{
							return true;
						}
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
						else if (notified_index == interruption_index)
						{
							detail::win32::ResetEvent(detail::get_current_thread_data()->interruption_handle);
							throw thread_interrupted();
						}
#endif
						else if (notified_index == timeout_index)
						{
							return false;
						}
					}
				}
				else
				{
					detail::win32::sleep(time_left.milliseconds);
				}
				if (target_time.relative)
				{
					target_time.milliseconds -= detail::timeout::max_non_infinite_wait;
				}
			} while (time_left.more);
			return false;
		}
	}
}