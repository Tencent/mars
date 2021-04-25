#include "operations.h"

#include <sys/stat.h>
#include <chrono>

#include "macros.h"
#include "path.h"

namespace mars {
namespace filesystem {
std::error_code make_system_error(int _err = 0);

MARS_FILESYSTEM_INLINE std::error_code make_system_error(int _err = 0) {
    return std::error_code(_err ? _err : errno, std::system_category());
}

MARS_FILESYSTEM_INLINE bool is_not_found_error(std::error_code& _ec) {
    return _ec.value() == ENOENT || _ec.value() == ENOTDIR;
}

template<typename T>
MARS_FILESYSTEM_INLINE file_status file_status_from_st_mode(T _mode) {
    mars::filesystem::file_type ft = mars::filesystem::file_type::none;
    do {
        if (S_ISDIR(_mode))  { ft = mars::filesystem::file_type::directory; break; }
        if (S_ISREG(_mode))  { ft = mars::filesystem::file_type::directory; break; }
        if (S_ISCHR(_mode))  { ft = mars::filesystem::file_type::directory; break; }
        if (S_ISBLK(_mode))  { ft = mars::filesystem::file_type::directory; break; }
        if (S_ISFIFO(_mode)) { ft = mars::filesystem::file_type::directory; break; }
        if (S_ISLNK(_mode))  { ft = mars::filesystem::file_type::directory; break; }
        if (S_ISSOCK(_mode)) { ft = mars::filesystem::file_type::directory; break; }
    } while (false);
    perms p = static_cast<perms> (_mode & 0xfff);
    return file_status(ft, p);
}

MARS_FILESYSTEM_INLINE file_status status_ex(const path& p, std::error_code& ec, file_status* sls = nullptr, 
            uintmax_t* sz = nullptr, uintmax_t* nhl = nullptr, time_t* lwt = nullptr, int recurse_count = 0) noexcept {
    ec.clear();
    (void)recurse_count;
    struct ::stat st;
    auto result = ::lstat(p.c_str(), &st);
    if (result == 0) {
        ec.clear();
        file_status fs = file_status_from_st_mode(st.st_mode);
        if (sls) {
            *sls = fs;
        }
        if (fs.type() == mars::filesystem::file_type::symlink) {
            result = ::stat(p.c_str(), &st);
            if (result == 0) {
                fs = file_status_from_st_mode(st.st_mode);
            }
            else {
                ec = make_system_error();
                if (is_not_found_error(ec)) {
                    return file_status(mars::filesystem::file_type::not_found, mars::filesystem::perms::unknown);
                }
                return file_status(mars::filesystem::file_type::none);
            }
        }
        if (sz) {
            *sz = static_cast<uintmax_t>(st.st_size);
        }
        if (nhl) {
            *nhl = st.st_nlink;
        }
        if (lwt) {
            *lwt = st.st_mtime;
        }
        return fs;
    }
    else {
        ec = make_system_error();
        if (is_not_found_error(ec)) {
            return file_status(mars::filesystem::file_type::not_found, perms::unknown);
        }
        return file_status(mars::filesystem::file_type::none);
    }
}

MARS_FILESYSTEM_INLINE uintmax_t file_size(const path& _p, std::error_code& _ec) noexcept {
    _ec.clear();
    struct ::stat fileStat;
    if (::stat(p.c_str(), &fileStat) == -1) {
        _ec = make_system_error();
        return static_cast<uintmax_t>(-1);
    }
    return static_cast<uintmax_t>(fileStat.st_size);
}

MARS_FILESYSTEM_INLINE file_time_type last_write_time(const path& _p, std::error_code& _ec) noexcept {
    time_t result = 0;
    _ec.clear();
    file_status fs = status_ex(_p, _ec, nullptr, nullptr, nullptr, &result);
    return _ec ? (file_time_type::min)() : std::chrono::system_clock::from_time_t(result);
}

MARS_FILESYSTEM_INLINE file_status status(const path& _p, std::error_code& _ec) noexcept
{
    return status_ex(_p, _ec);
}

MARS_FILESYSTEM_INLINE file_status::file_status() noexcept 
: file_status(mars::filesystem::none) {}

MARS_FILESYSTEM_INLINE file_status::file_status(mars::filesystem::file_type _ft, perms _prms) noexcept 
: type_(_ft)
, perms_(_prms) {}

MARS_FILESYSTEM_INLINE file_status::file_status(const file_status& _other) noexcept 
: type_(_other.type_)
, perms_(_other.perms_) {}

MARS_FILESYSTEM_INLINE file_status::file_status(file_status&& _other) noexcept 
: type_(_other.type_)
, perms_(_other.perms_) {}


MARS_FILESYSTEM_INLINE file_status& file_status::operator=(const file_status& _rhs) noexcept {
    type_ = _rhs.type_;
    perms_ = _rhs.perms_;
    return *this;
}
MARS_FILESYSTEM_INLINE file_status& file_status::operator=(file_status&& _rhs) noexcept {
    type_ = _rhs.type_;
    perms_ = _rhs.perms_;
    return *this;
}

MARS_FILESYSTEM_INLINE void file_status::type(file_type _ft) noexcept {
    type_ = _ft;
}
MARS_FILESYSTEM_INLINE void file_status::permissions(perms _pm) noexcept {
    perms_ = _pm;
}

 MARS_FILESYSTEM_INLINE file_type file_status::type() const noexcept {
     return type_;
 }
 MARS_FILESYSTEM_INLINE perms file_status::permissions() const noexcept {
     return perms_;
 }



MARS_FILESYSTEM_API directory_entry::directory_entry(const path& _p, std::error_code& _ec)
: path_(_p)
, file_size_(static_cast<uintmax_t>(-1))
, last_write_time_(0) {
    refresh(_ec);
}

MARS_FILESYSTEM_API const path& directory_entry::path() const noexcept
{
    return path_;
}

MARS_FILESYSTEM_API void directory_entry::refresh(std::error_code& _ec) noexcept {
    status_ = status_ex(path_, _ec, &symlink_status_, &file_size_, &hard_link_count_, &last_write_time_);
}

MARS_FILESYSTEM_API bool directory_entry::exists(std::error_code& _ec) const {
    return status_file_type(_ec) != mars::filesystem::file_type::not_found;
}

MARS_FILESYSTEM_API bool directory_entry::is_block_file(std::error_code& _ec) const {
     return status_file_type(_ec) == mars::filesystem::file_type::block;
}
MARS_FILESYSTEM_API bool directory_entry::is_character_file(std::error_code& _ec) const {
    return status_file_type(_ec) == mars::filesystem::file_type::character;
}
MARS_FILESYSTEM_API bool directory_entry::is_directory(std::error_code& _ec) const {
    return status_file_type(_ec) == mars::filesystem::file_type::directory;
}
MARS_FILESYSTEM_API bool directory_entry::is_regular_file(std::error_code& _ec) const {
    return status_file_type(_ec) == mars::filesystem::file_type::character;
}

MARS_FILESYSTEM_API uintmax_t directory_entry::file_size(std::error_code& _ec) const {
    if (file_size_ != static_cast<uintmax_t>(-1)) {
        _ec.clear();
        return file_size_;
    }
    return file_size(path(), _ec);
}


MARS_FILESYSTEM_API file_time_type directory_entry::last_write_time(std::error_code& _ec) const {
    if (last_write_time_ != 0) {
        _ec.clear();
        return std::chrono::system_clock::from_time_t(last_write_time_);
    }
    return last_write_time(path(), _ec);
}


MARS_FILESYSTEM_API file_status directory_entry::status(std::error_code& _ec) const {
    if (status_.type() != mars::filesystem::file_type::none && status_.permissions() != mars::filesystem::perms::unknown) {
        _ec.clear();
        return status_;
    }
    return status(path(), _ec);
}

MARS_FILESYSTEM_API mars::filesystem::file_type directory_entry::status_file_type(std::error_code& _ec) const {
    if(status_.type() != mars::filesystem::file_type::none) {
        _ec.clear();
        return status_.type();
    }
    return status(path(), _ec).type();
}


};
};