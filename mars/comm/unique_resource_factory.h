#pragma once

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "unique_resource.h"

class UniqueResourceFactory {
 public:
    static UniqueSocketResource CreateSocket(int domain, int type, int protocol);
    static UniqueFDResource OpenFileReadOnly(const std::string& filepath);
    static UniqueFDResource OpenFileUpdate(const std::string& filepath);
    static UniqueFDResource CreateNewFile(const std::string& filepath, uint64_t reserve_bytes = 0);
    static UniqueFileResource OpenPFileReadOnly(const std::string& filepath);
    static UniqueFileResource OpenPFileUpdate(const std::string& filepath);
    static UniqueFileResource CreateNewPFile(const std::string& filepath);
};
