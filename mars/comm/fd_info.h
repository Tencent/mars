//
//  fd_info.hpp
//  cdn
//
//  Created by perryzhou on 2019/1/4.
//

#pragma once

#include <list>
#include <string>

namespace mars {
namespace comm {
struct FDI {
    int fd = 0;
    int type = 0;
    int error = 0;
    std::string path_or_name;

    bool IsSocket() const;
    bool IsFile() const;
    bool IsPipe() const;
};

class FDInfo {
 public:
    static FDI QueryFD(int fd);
    static std::list<FDI> QueryFDInfo(int maxfd);
    static std::list<std::string> PrettyFDInfo(const std::list<FDI>& fdi);
};

}  // namespace comm
};  // namespace mars
