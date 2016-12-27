#if UWP
# include <windows.h>
#include <atlbase.h>
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100


static wchar_t* toWcsForPath(const char* src, int len)
{
	if (!len)
	{
		return NULL;
	}
	wchar_t *dst = new wchar_t[len +1];

	for(int i=0 ; i < len ; i++)
	{
		dst[i] = src[i];
	}
	dst[len] = 0;
	return dst;

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
	para.lpSecurityAttributes = lpSecurityAttributes;
	para.hTemplateFile = hTemplateFile;
	para.dwFileAttributes = dwFlagsAndAttributes;
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



#endif 

