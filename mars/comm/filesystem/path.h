#ifndef MARS_FILE_SYSTEM_PATH_H_
#define MARS_FILE_SYSTEM_PATH_H_

#include "macros.h"
#include <string>


namespace mars {
namespace filesystem {

typedef std::string::size_type size_type;
typedef std::string string_type;
class MARS_FILESYSTEM_API path {

public:
    path() noexcept;
    path(const path& _p);
    path(path&& _p) noexcept;
    path(string_type&& _path);
    template<class InputIterator>
    path(InputIterator first, InputIterator last);
    ~path();

    path& operator=(const path& _p);
    path& operator=(path&& _p) noexcept;

    path root_name() const;
    path root_directory() const;
    path root_path() const;
    path filename() const;
    path stem() const;
    path extension() const;

    bool empty() const noexcept;
    bool has_root_name() const;
    bool has_root_directory() const;
    bool has_root_path() const;
    bool has_filename() const;
    bool has_stem() const;
    bool has_extension() const;
    // bool is_absolute() const;
    // bool is_relative() const;

    size_type root_directory_start(const string_type& _path, size_type _size) const;

    const char* c_str() {return m_pathname.c_str();}
    string_type string() const;


private:
    string_type m_pathname;

};


}; //namespace filesystem
};  //namespace mars

#endif