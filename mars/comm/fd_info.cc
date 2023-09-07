//
//  fd_info.cpp
//  cdn
//
//  Created by perryzhou on 2019/1/4.
//

#include "fd_info.h"
#include <string.h>

#ifndef WIN32
#include <errno.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

static bool get_fd_path(int fd, char szbuf[1024]) {
#ifdef __APPLE__
    return -1 != fcntl(fd, F_GETPATH, szbuf);
#endif

#ifdef __ANDROID__
    char path[64];
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
    ssize_t length = ::readlink(path, szbuf, 1023);
    if (length > 0) {
        szbuf[length] = '\0';
        return true;
    }
#endif

    return false;
}

namespace mars {
namespace comm {

bool FDI::IsSocket() const {
#ifndef WIN32
    return type == S_IFSOCK;
#else
    return true;
#endif
}
bool FDI::IsFile() const {
#ifndef WIN32
    return type == S_IFREG;
#else
    return true;
#endif
}
bool FDI::IsPipe() const {
#ifndef WIN32
    return type == S_IFIFO;
#else
    return true;
#endif
}

FDI FDInfo::QueryFD(int fd) {
    FDI item;
    item.fd = fd;

#ifndef WIN32
    int flags = fcntl(fd, F_GETFD, 0);
    if (-1 == flags) {
        item.error = errno;
        item.path_or_name = "<F_GETFD failed>";
    } else {
        struct stat statbuf;
        if (fstat(fd, &statbuf) == 0) {
            item.type = (S_IFMT & statbuf.st_mode);
            item.path_or_name = "<unknown>";

            char szbuf[1024] = {0};
            if (get_fd_path(fd, szbuf)) {
                const char* path = strrchr(szbuf, '/');
                if (path != nullptr) {
                    item.path_or_name = path;
                } else {
                    item.path_or_name = szbuf;
                }
            }
        }
    }
#endif
    return item;
}

std::list<FDI> FDInfo::QueryFDInfo(int maxfd) {
    std::list<FDI> result;
    for (int fd = 0; fd < maxfd; fd++) {
        result.push_back(QueryFD(fd));
    }
    return result;
}

static const char* type2name(int type) {
#ifndef WIN32
    switch (type) {
        case S_IFIFO:
            return "named pipe";
        case S_IFCHR:
            return "character special";
        case S_IFDIR:
            return "directory";
        case S_IFBLK:
            return "block special";
        case S_IFREG:
            return "regular";
        case S_IFLNK:
            return "symbolic link";
        case S_IFSOCK:
            return "socket";
        default:
            return "<unknown>";
    }
#else
    return "<win32.unkonwn>";
#endif
}

std::list<std::string> FDInfo::PrettyFDInfo(const std::list<FDI>& fdi) {
    std::list<std::string> result;
#ifndef WIN32
    char szline[1024];
    std::string part;
    for (auto item : fdi) {
        if (part.length() >= 8192) {
            result.push_back(part);
            part.clear();
        }
        size_t rv = snprintf(szline,
                             1024,
                             "\r\n%d|%d|%d(%s)|%s",
                             item.fd,
                             item.error,
                             item.type,
                             type2name(item.type),
                             item.path_or_name.c_str());
        part.append(szline, rv);
    }
    result.push_back(part);
#endif

    return result;
}

}  // namespace comm
};  // namespace mars
