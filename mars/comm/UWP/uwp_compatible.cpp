#if UWP
# include <windows.h>

#include <boost/thread/win32/thread_data.hpp>
#include <network/getgateway.h>
#include <network/getifaddrs.h>
//#include <atlbase.h>
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100


static wchar_t* toWcsForPath(const char* src, int len)
{
	UINT charset = CP_ACP;
//#if defined(WIN32)
//	charset = CP_UTF8;//uwp using CP_ACP
//#endif

	wchar_t* wszRet;
	int size = MultiByteToWideChar(charset, 0, (char*)src, len, NULL, 0);
	if (size <= 0)
		return NULL;
	wszRet = new wchar_t[size + 1];
	memset(wszRet, 0, (size + 1) * sizeof(wchar_t));
	MultiByteToWideChar(charset, 0, (char*)src, len, wszRet, size);
	return wszRet;
}




DWORD SetFilePointer(
	_In_ HANDLE hFile,
	_In_ LONG lDistanceToMove,
	_Inout_opt_ PLONG lpDistanceToMoveHigh,
	_In_ DWORD dwMoveMethod
	)
{
	LARGE_INTEGER liDistanceToMove = { lDistanceToMove };
	LARGE_INTEGER lpNewFilePointer = { *lpDistanceToMoveHigh };
	return ::SetFilePointerEx(hFile, liDistanceToMove, &lpNewFilePointer, dwMoveMethod);
}


//WINBASEAPI
//HANDLE
//WINAPI
//CreateFile2(
//	_In_ LPCWSTR lpFileName,
//	_In_ DWORD dwDesiredAccess,
//	_In_ DWORD dwShareMode,
//	_In_ DWORD dwCreationDisposition,
//	_In_opt_ LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams
//	)
//{
//
//}


//typedef struct _CREATEFILE2_EXTENDED_PARAMETERS {
//	DWORD dwSize;
//	DWORD dwFileAttributes;
//	DWORD dwFileFlags;
//	DWORD dwSecurityQosFlags;
//	LPSECURITY_ATTRIBUTES lpSecurityAttributes;
//	HANDLE hTemplateFile;
//} CREATEFILE2_EXTENDED_PARAMETERS, *PCREATEFILE2_EXTENDED_PARAMETERS, *LPCREATEFILE2_EXTENDED_PARAMETERS;

HANDLE
CreateFileW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,//
	_In_opt_ HANDLE hTemplateFile
	)
{
	CREATEFILE2_EXTENDED_PARAMETERS para;
	para.lpSecurityAttributes = lpSecurityAttributes;
	para.hTemplateFile = hTemplateFile;
	para.dwFileAttributes = dwFlagsAndAttributes;
	return CreateFile2(lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, &para);
}




DWORD GetFileSize(
	_In_      HANDLE  hFile,
	_Out_opt_ LPDWORD lpFileSizeHigh
	) {
	// _In_ HANDLE hFile,
   // _Out_ PLARGE_INTEGER lpFileSize
		//);
	LARGE_INTEGER nSize = { 0 };
	bool bSuc = ::GetFileSizeEx(hFile, &nSize);
	if (bSuc)
	{
		*lpFileSizeHigh = nSize.HighPart;
		return nSize.LowPart;
	}
	return INVALID_FILE_SIZE;
}


HANDLE
WINAPI
CreateFileA(
	_In_ LPCSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
	)
{
	CREATEFILE2_EXTENDED_PARAMETERS para;
	memset(&para, 0, sizeof(CREATEFILE2_EXTENDED_PARAMETERS));
	para.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
	para.lpSecurityAttributes = lpSecurityAttributes;
	para.hTemplateFile = hTemplateFile;
	para.dwFileFlags = dwFlagsAndAttributes & 0xFFFF0000;
	para.dwFileAttributes = dwFlagsAndAttributes & 0xFFFFF;
	wchar_t * strName = toWcsForPath(lpFileName, lpFileName? strlen(lpFileName) : 0);
	HANDLE hRet = CreateFile2(strName, dwDesiredAccess, dwShareMode, dwCreationDisposition, &para);
	delete[] strName;
	return hRet;
}

//WINAPI
//CreateFileMappingA(
//	_In_     HANDLE hFile,
//	_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
//	_In_     DWORD flProtect,
//	_In_     DWORD dwMaximumSizeHigh,
//	_In_     DWORD dwMaximumSizeLow,
//	_In_opt_ LPCSTR lpName
//	);

HANDLE
CreateFileMappingA(
	_In_     HANDLE hFile,
	_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
	_In_     DWORD flProtect,
	_In_     DWORD dwMaximumSizeHigh,
	_In_     DWORD dwMaximumSizeLow,
	_In_opt_ LPCSTR lpName
	) {

	/*FORCEINLINE
_Ret_maybenull_
HANDLE
WINAPI
CreateFileMappingW(
    _In_     HANDLE hFile,
    _In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    _In_     DWORD flProtect,
    _In_     DWORD dwMaximumSizeHigh,
    _In_     DWORD dwMaximumSizeLow,
    _In_opt_ LPCWSTR lpName
    )*/

	wchar_t * strName = toWcsForPath(lpName, lpName ? strlen(lpName) : 0);
	HANDLE hTemp =::CreateFileMapping(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, strName);
	delete[] strName;
	return hTemp;
}



LPVOID
WINAPI
MapViewOfFileEx(
	_In_ HANDLE hFileMappingObject,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwFileOffsetHigh,
	_In_ DWORD dwFileOffsetLow,
	_In_ SIZE_T dwNumberOfBytesToMap,
	_In_opt_ LPVOID lpBaseAddress
	)
{

	/*WINBASEAPI
		MapViewOfFile(
    _In_ HANDLE hFileMappingObject,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwFileOffsetHigh,
    _In_ DWORD dwFileOffsetLow,
    _In_ SIZE_T dwNumberOfBytesToMap
    )*/
	//dwFileOffsetHigh
	return ::MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
}


//empty ipv6 implement
int getdefaultgateway(struct in_addr * addr)
{
	return 0;
}

int getdefaultgateway6(struct in6_addr * addr)
{
	return 0;
}


bool getifaddrs_ipv4_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter)
{
	return false;
}

bool getifaddrs_ipv6_filter(std::vector<ifaddrinfo_ip_t>& _addrs, unsigned int _flags_filter)
{
	return false;
}


using namespace boost;
namespace mars_boost
{
	namespace this_thread
	{
		bool interruptible_wait(detail::win32::handle handle_to_wait_for, detail::timeout target_time)
		{
			detail::win32::handle handles[3] = { 0 };
			unsigned handle_count = 0;
			unsigned wait_handle_index = ~0U;

			unsigned timeout_index = ~0U;
			if (handle_to_wait_for != detail::win32::invalid_handle_value)
			{
				wait_handle_index = handle_count;
				handles[handle_count++] = handle_to_wait_for;
			}
			detail::win32::handle_manager timer_handle;

			detail::timeout::remaining_time time_left(0);

			do
			{
				time_left = target_time.remaining_milliseconds();

				if (handle_count)
				{
					unsigned long const notified_index = ::WaitForMultipleObjectsEx(handle_count, (const HANDLE*)handles, false, time_left.milliseconds, false); //((DWORD)handle_count,(const HANDLE*)handles,false,(DWORD)time_left.milliseconds);
					if ((notified_index - WAIT_OBJECT_0)<handle_count)
					{
						if ((notified_index - WAIT_OBJECT_0) == wait_handle_index)
						{
							return true;
						}
					}
					else if (notified_index == WAIT_TIMEOUT)
						return false;
					else if (notified_index == WAIT_FAILED)
					{
						int err = GetLastError();
						return false;
					}
				}
				else
				{
#ifdef WP8
					std::chrono::milliseconds dura(time_left.milliseconds);
					std::this_thread::sleep_for(dura);
#else
					detail::win32::Sleep(time_left.milliseconds);
#endif

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


#endif 

