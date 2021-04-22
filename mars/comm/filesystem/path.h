#ifndef MARS_FILE_SYSTEM_PATH_H_
#define MARS_FILE_SYSTEM_PATH_H_

#include "macros.h"

namespace mars {
namespace filesystem {

class path {
public:
    path() noexcept;
    path(const path& _p);
    path(path&& _p) noexcept;
    ~path();

    path& operator=(const path& _p);
    path& operator=(path&& _p) noexcept;

    path root_name() const;
    path root_directory() const;
    path root_path() const;
    path relative_path() const;
    path filename() const;
    path stem() const;
    path extension();

    bool empty() const noexcept;
    bool has_root_name() const;
    bool has_root_directory() const;
    bool has_root_path() const;
    bool has_relative_path() const;
    bool has_filename() const;
    bool has_stem() const;
    bool has_extension() const;
    bool is_absolute() const;
    bool is_relative() const;

};

MARS_FILESYSTEM_INLINE path root_name() const {

}

}; //namespace filesystem
};  //namespace mars

#endif