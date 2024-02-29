//  Boost reparse_tag_file_placeholder.cpp  ---------------------------------------------------------//

//  Copyright Roman Savchenko 2020

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

#include <iostream>

#if defined(BOOST_FILESYSTEM_HAS_MKLINK)

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/core/lightweight_test.hpp>

#include <cstddef>
#include <exception>

#include <windows.h>
#include <winnt.h>

#ifdef _MSC_VER
#pragma comment(lib, "Advapi32.lib")
#endif

// Test correct mars_boost::filesystem::status when reparse point ReparseTag set to IO_REPARSE_TAG_FILE_PLACEHOLDER
// https://docs.microsoft.com/en-us/windows/compatibility/placeholder-files?redirectedfrom=MSDN

#if !defined(__MINGW32__) || defined(__MINGW64__)
typedef struct _REPARSE_DATA_BUFFER
{
    ULONG ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union
    {
        struct
        {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG Flags;
            WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct
        {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } MountPointReparseBuffer;
        struct
        {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
#endif

#ifndef IO_REPARSE_TAG_FILE_PLACEHOLDER
#define IO_REPARSE_TAG_FILE_PLACEHOLDER (0x80000015L)
#endif

#ifndef FSCTL_SET_REPARSE_POINT
#define FSCTL_SET_REPARSE_POINT (0x000900a4)
#endif

#ifndef REPARSE_DATA_BUFFER_HEADER_SIZE
#define REPARSE_DATA_BUFFER_HEADER_SIZE FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)
#endif

bool obtain_restore_privilege()
{
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        DWORD err = GetLastError();
        std::cout << "OpenProcessToken() failed with: " << err << std::endl;
        return false;
    }

    TOKEN_PRIVILEGES tp;
    if (!LookupPrivilegeValue(NULL, SE_RESTORE_NAME, &tp.Privileges[0].Luid))
    {
        DWORD err = GetLastError();
        CloseHandle(hToken);
        std::cout << "LookupPrivilegeValue() failed with: " << err << std::endl;
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
        DWORD err = GetLastError();
        CloseHandle(hToken);
        std::cout << "AdjustTokenPrivileges() failed with: " << err << std::endl;
        return false;
    }

    CloseHandle(hToken);
    return true;
}

bool create_io_reparse_file_placeholder(const wchar_t* name)
{
    if (!obtain_restore_privilege())
    {
        return false;
    }

    HANDLE hHandle = CreateFileW(name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_OPEN_REPARSE_POINT, 0);
    if (hHandle == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        std::cout << "CreateFile() failed with: " << err << std::endl;
        return false;
    }

    PREPARSE_DATA_BUFFER pReparse = reinterpret_cast< PREPARSE_DATA_BUFFER >(GlobalAlloc(GPTR, MAXIMUM_REPARSE_DATA_BUFFER_SIZE));
    if (!pReparse)
    {
        DWORD err = GetLastError();
        CloseHandle(hHandle);
        std::cout << "GlobalAlloc() failed with: " << err << std::endl;
        return false;
    }
    //note: IO_REPARSE_TAG_FILE_PLACEHOLDER - just to show that reparse point could be not only symlink or junction
    pReparse->ReparseTag = IO_REPARSE_TAG_FILE_PLACEHOLDER;

    DWORD dwLen;
    bool ret = !!DeviceIoControl(hHandle, FSCTL_SET_REPARSE_POINT, pReparse, pReparse->ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE, NULL, 0, &dwLen, NULL);
    if (!ret)
    {
        DWORD err = GetLastError();
        std::cout << "DeviceIoControl() failed with: " << err << std::endl;
    }

    CloseHandle(hHandle);
    GlobalFree(pReparse);
    return ret;
}

int main()
{
    mars_boost::filesystem::path rpt = mars_boost::filesystem::temp_directory_path() / "reparse_point_test.txt";

    BOOST_TEST(create_io_reparse_file_placeholder(rpt.native().c_str()));
    std::cout << "Created file placeholder reparse point: " << rpt.string() << std::endl;
    BOOST_TEST_NO_THROW(BOOST_TEST(mars_boost::filesystem::status(rpt).type() == mars_boost::filesystem::reparse_file));
    BOOST_TEST_NO_THROW(BOOST_TEST(mars_boost::filesystem::remove(rpt)));

    return mars_boost::report_errors();
}

#else // defined(BOOST_FILESYSTEM_HAS_MKLINK)

int main()
{
    std::cout << "Skipping test as the target system does not support mklink." << std::endl;
    return 0;
}

#endif // defined(BOOST_FILESYSTEM_HAS_MKLINK)
