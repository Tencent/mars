#include "unique_resource_factory.h"

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#include <stddef.h>
#include <stringapiset.h>

namespace internal {
std::wstring string2wstring(const std::string& input) {
    size_t nLen = input.length();
    if (nLen == 0)
        return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)input.c_str(), (int)nLen, NULL, 0);
    if (size <= 0)
        return L"";

    std::wstring wstr;
    wstr.resize(size + 1);

    int nResult = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)input.c_str(), (int)nLen, (LPWSTR)&wstr[0], size);
    if (nResult == 0) {
        return L"";
    }

    wstr.resize(nResult);
    return wstr;
}
}  // namespace internal

#define READFLAG (_O_BINARY | _O_RDONLY)
#define WRITEFLAG (_O_BINARY | _O_CREAT | _O_RDWR)
#define WRITENEWFLAG (_O_BINARY | _O_CREAT | _O_RDWR | _O_TRUNC)
#define FILEMODE (_S_IREAD | _S_IWRITE)

#else

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define READFLAG (O_RDONLY)
#define WRITEFLAG (O_CREAT | O_RDWR | O_SYNC)
#define WRITENEWFLAG (O_CREAT | O_RDWR | O_SYNC | O_TRUNC)
#define FILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#endif

namespace internal {
int OpenFile(const std::string& file, int flag, int mode) {
#ifdef WIN32
    std::wstring wpath = string2wstring(file);
    int fd = _wopen(wpath.c_str(), flag, mode);
#else
    int fd = open(file.c_str(), flag, mode);
#endif
    if (fd == -1) {
        xerror2(TSF "path:%_, flag:%_, mode:%_, err:%_", file, flag, mode, errno);
    }
    return fd;
}

#ifdef WIN32
#define READFLAG (_O_BINARY | _O_RDONLY)
#define WRITEFLAG (_O_BINARY | _O_CREAT | _O_RDWR)
#define WRITENEWFLAG (_O_BINARY | _O_CREAT | _O_RDWR | _O_TRUNC)
#define FILEMODE (_S_IREAD | _S_IWRITE)
#else
#define READFLAG (O_RDONLY)
#define WRITEFLAG (O_CREAT | O_RDWR | O_SYNC)
#define WRITENEWFLAG (O_CREAT | O_RDWR | O_SYNC | O_TRUNC)
#define FILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#endif

int OpenForRead(const std::string& file) {
    return OpenFile(file, READFLAG, FILEMODE);
}
int OpenForUpdate(const std::string& file) {
    return OpenFile(file, WRITEFLAG, FILEMODE);
}
int OpenForWriteNew(const std::string& file, uint64_t allocsize) {
    return OpenFile(file, WRITENEWFLAG, FILEMODE);
}
};  // namespace internal

UniqueSocketResource UniqueResourceFactory::CreateSocket(int domain, int type, int protocol) {
    return UniqueSocketResource(socket(domain, type, protocol));
}
UniqueFDResource UniqueResourceFactory::OpenFileReadOnly(const std::string& filepath) {
    return UniqueFDResource(internal::OpenForRead(filepath));
}
UniqueFDResource UniqueResourceFactory::OpenFileUpdate(const std::string& filepath) {
    return UniqueFDResource(internal::OpenForUpdate(filepath));
}
UniqueFDResource UniqueResourceFactory::CreateNewFile(const std::string& filepath, uint64_t reserve_bytes /*=0*/) {
    return UniqueFDResource(internal::OpenForWriteNew(filepath, reserve_bytes));
}
UniqueFileResource UniqueResourceFactory::OpenPFileReadOnly(const std::string& filepath) {
    return UniqueFileResource(fopen(filepath.c_str(), "rb"));
}
UniqueFileResource UniqueResourceFactory::OpenPFileUpdate(const std::string& filepath) {
    return UniqueFileResource(fopen(filepath.c_str(), "wb"));
}
UniqueFileResource UniqueResourceFactory::CreateNewPFile(const std::string& filepath) {
    return UniqueFileResource(fopen(filepath.c_str(), "wb+"));
}
