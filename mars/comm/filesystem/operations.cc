#include "operations.h"

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <chrono>
#include <unistd.h> 

#include "macros.h"
#include "path.h"


namespace mars {
namespace filesystem {

using sec = std::error_code;

MARS_FILESYSTEM_INLINE sec make_system_error(int _err) {
    return sec(_err ? _err : errno, std::system_category());
}

MARS_FILESYSTEM_INLINE bool is_not_found_error(sec& _ec) {
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

MARS_FILESYSTEM_INLINE file_status status_ex(const path& p, sec& ec, file_status* sls = nullptr, 
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

MARS_FILESYSTEM_INLINE uintmax_t file_size(const path& _p, sec& _ec) noexcept {
    _ec.clear();
    struct ::stat fileStat;
    if (::stat(_p.c_str(), &fileStat) == -1) {
        _ec = make_system_error();
        return static_cast<uintmax_t>(-1);
    }
    return static_cast<uintmax_t>(fileStat.st_size);
}

MARS_FILESYSTEM_INLINE intmax_t file_size(const path& _p) {
    sec ec;
    return file_size(_p, ec);
}

MARS_FILESYSTEM_INLINE file_time_type last_write_time(const path& _p, sec& _ec) noexcept {
    time_t result = 0;
    _ec.clear();
    file_status fs = status_ex(_p, _ec, nullptr, nullptr, nullptr, &result);
    return _ec ? (file_time_type::min)() : std::chrono::system_clock::from_time_t(result);
}

MARS_FILESYSTEM_INLINE file_time_type last_write_time(const path& _p) {
    sec ec;
    return last_write_time(_p, ec);
}

MARS_FILESYSTEM_INLINE bool remove(const path& p, sec& ec) noexcept {
    ec.clear();
    if (::remove(p.c_str()) == -1) {
        auto error = errno;
        if (error == ENOENT) {
            return false;
        }
        ec = make_system_error();
    }
    return ec ? false : true;
}

MARS_FILESYSTEM_INLINE bool remove(const path& p) {
    sec ec;
    return remove(p, ec);
}

MARS_FILESYSTEM_INLINE file_status status(const path& _p) {
    sec ec;
    return status_ex(_p, ec);
}

MARS_FILESYSTEM_INLINE bool is_directory(file_status s) {
    return s.type() == mars::filesystem::file_type::directory;
}

MARS_FILESYSTEM_INLINE bool is_directory(const path& _p) {
    sec ec;
    return is_directory(status(_p));
}

MARS_FILESYSTEM_INLINE bool is_regular_file(file_status s) {
    return s.type() == mars::filesystem::file_type::regular;
}

MARS_FILESYSTEM_INLINE bool is_regular_file(const path& p) {
    return is_regular_file(status(p));
}

MARS_FILESYSTEM_INLINE bool is_symlink(file_status s) noexcept {
    return s.type() == file_type::symlink;
}

MARS_FILESYSTEM_INLINE bool exists(file_status s) noexcept {
    return s.type() != mars::filesystem::file_type::none && s.type() != mars::filesystem::file_type::not_found;
}

MARS_FILESYSTEM_INLINE bool exists(const path& p) {
    return exists(status(p));
}

MARS_FILESYSTEM_INLINE bool exists(const path& p, std::error_code& ec) noexcept {
    file_status s = status_ex(p, ec);
    if (s.type() != file_type::none) {
        ec.clear();
    }
    return exists(status(p));
}

MARS_FILESYSTEM_INLINE uintmax_t remove_all(const path& p, std::error_code& ec) noexcept
{
    ec.clear();
    uintmax_t count = 0;
    if (p == "/") {
        ec = make_system_error(-5);
        return static_cast<uintmax_t>(-1);
    }
    auto fs = status(p);
    if (exists(fs) && is_directory(fs)) {
        for (auto iter = directory_iterator(p); iter != directory_iterator(); ++iter) {
            if (ec && !is_not_found_error(ec)) {
                break;
            }
            bool is_symlink_result = iter->is_symlink();
            if (ec)
                return static_cast<uintmax_t>(-1);
            if (!is_symlink_result && iter->is_directory()) {
                count += remove_all(iter->path(), ec);
                if (ec) {
                    return static_cast<uintmax_t>(-1);
                }
            }
            else {
                if (!ec) {
                    remove(iter->path(), ec);
                }
                if (ec) {
                    return static_cast<uintmax_t>(-1);
                }
                ++count;
            }
        }
    }
    if (!ec) {
        if (remove(p, ec)) {
            ++count;
        }
    }
    if (ec) {
        return static_cast<uintmax_t>(-1);
    }
    return count;
}

MARS_FILESYSTEM_INLINE uintmax_t remove_all(const path& p) {
    std::error_code ec;
    return remove_all(p, ec);
}


MARS_FILESYSTEM_INLINE space_info space(const path& p) {
    
    struct ::statvfs sfs;
    if (::statvfs(p.c_str(), &sfs) != 0) {
        return {static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1)};
    }
    return {static_cast<uintmax_t>(sfs.f_blocks * sfs.f_frsize), static_cast<uintmax_t>(sfs.f_bfree * sfs.f_frsize), static_cast<uintmax_t>(sfs.f_bavail * sfs.f_frsize)};
}


MARS_FILESYSTEM_INLINE bool create_directory(const path& p) {
    sec ec;
    return create_directory(p, ec);
}

MARS_FILESYSTEM_INLINE bool create_directory(const path& p, std::error_code& ec) noexcept {
    ::mode_t attribs = static_cast<mode_t>(perms::all);
    // if (!path().empty()) {
    //     struct ::stat fileStat;
    //     if (::stat(path().c_str(), &fileStat) != 0) {
    //         ec = make_system_error();
    //         return false;
    //     }
    //     attribs = fileStat.st_mode;
    // }
    if (::mkdir(p.c_str(), attribs) != 0) {
        ec = make_system_error();
        return false;
    }
}


MARS_FILESYSTEM_INLINE bool create_directories(const path& _p) {
    sec ec;
    return create_directories(_p, ec);
}

MARS_FILESYSTEM_INLINE bool create_directories(const path& _p, std::error_code& _ec) noexcept {
    path current;
    bool didCreate = false;
    _ec.clear();
    for (path::iterator part =_p.begin(); part != _p.end(); part ++) {
        current /= *part;
        if (current != _p.root_name() && current != _p.root_path()) {
            std::error_code tec;
            auto fs = status_ex(current, tec);
            if (tec && fs.type() != mars::filesystem::file_type::not_found) {
                _ec = tec;
                return false;
            }
            if (!exists(fs)) {
                create_directory(current, _ec);
                if (!is_directory(current)) {
                    return false;
                }
                didCreate = true;
            }
        }
    }
    return didCreate;
}


MARS_FILESYSTEM_INLINE void resize_file(const path& p, uintmax_t size, std::error_code& ec) noexcept {
    if (::truncate(p.c_str(), static_cast<off_t>(size)) != 0) {
        ec = make_system_error();
    }
}



MARS_FILESYSTEM_INLINE file_status::file_status() noexcept 
: type_(mars::filesystem::file_type::none)
, perms_(mars::filesystem::perms::none){}

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



MARS_FILESYSTEM_API directory_entry::directory_entry(const mars::filesystem::path& _p, sec& _ec)
: path_(_p)
, file_size_(static_cast<uintmax_t>(-1))
, last_write_time_(0) {
    refresh();
}

MARS_FILESYSTEM_API const path& directory_entry::path() const noexcept
{
    return path_;
}

MARS_FILESYSTEM_API void directory_entry::refresh() noexcept {
    sec ec;
    status_ = status_ex(path_, ec, &symlink_status_, &file_size_, &hard_link_count_, &last_write_time_);
}

MARS_FILESYSTEM_API bool directory_entry::exists() const {
    return status_file_type() != mars::filesystem::file_type::not_found;
}

MARS_FILESYSTEM_API bool directory_entry::is_block_file() const {
     return status_file_type() == mars::filesystem::file_type::block;
}
MARS_FILESYSTEM_API bool directory_entry::is_character_file() const {
    return status_file_type() == mars::filesystem::file_type::character;
}
MARS_FILESYSTEM_API bool directory_entry::is_directory() const {
    return status_file_type() == mars::filesystem::file_type::directory;
}
MARS_FILESYSTEM_API bool directory_entry::is_regular_file() const {
    return status_file_type() == mars::filesystem::file_type::character;
}

MARS_FILESYSTEM_API uintmax_t directory_entry::file_size() const {
    if (file_size_ != static_cast<uintmax_t>(-1)) {
        return file_size_;
    }
    sec ec;
    return mars::filesystem::file_size(path(), ec);
}


MARS_FILESYSTEM_API file_time_type directory_entry::last_write_time() const {
    if (last_write_time_ != 0) {
        return std::chrono::system_clock::from_time_t(last_write_time_);
    }
    sec ec;
    return mars::filesystem::last_write_time(path(), ec);
}


MARS_FILESYSTEM_API file_status directory_entry::status() const {
    if (status_.type() != mars::filesystem::file_type::none && status_.permissions() != mars::filesystem::perms::unknown) {
        return status_;
    }
    return mars::filesystem::status(path());
}

MARS_FILESYSTEM_API mars::filesystem::file_type directory_entry::status_file_type() const {
    if(status_.type() != mars::filesystem::file_type::none) {
        return status_.type();
    }
    return mars::filesystem::status(path()).type();
}

MARS_FILESYSTEM_API bool directory_entry::is_symlink() const {
    return symlink_status_.type() != file_type::none ? symlink_status_.type() == file_type::symlink : filesystem::is_symlink(symlink_status_);
}


MARS_FILESYSTEM_API directory_iterator::directory_iterator() noexcept
: impl_(new impl(path(), mars::filesystem::directory_options::none)) {}

MARS_FILESYSTEM_API directory_iterator::directory_iterator(const path& _p) noexcept
: impl_(new impl(_p, mars::filesystem::directory_options::none)) {
    impl_->ec_.clear();
}

MARS_FILESYSTEM_API directory_iterator::directory_iterator(const directory_iterator& _rhs) 
: impl_(_rhs.impl_) {}

MARS_FILESYSTEM_API directory_iterator::directory_iterator(const directory_iterator&& _rhs) noexcept
: impl_(std::move(_rhs.impl_)){}

MARS_FILESYSTEM_API directory_iterator& directory_iterator::operator=(const directory_iterator& _rhs) {
    impl_ = _rhs.impl_;
    return *this;
}
MARS_FILESYSTEM_API directory_iterator& directory_iterator::operator=(directory_iterator&& _rhs) noexcept {
    impl_ = std::move(_rhs.impl_);
    return *this;
}


MARS_FILESYSTEM_API directory_iterator& directory_iterator::operator++() {
    sec ec;
    impl_->increment(ec);
    return *this;
}

MARS_FILESYSTEM_API bool directory_iterator::operator==(const directory_iterator& _rhs) const {
    return impl_->dir_entry_.path_.string() == _rhs.impl_->dir_entry_.path_.string();
}

MARS_FILESYSTEM_API bool directory_iterator::operator!=(const directory_iterator& _rhs) const {
    return !(this->operator==(_rhs));
}
MARS_FILESYSTEM_API const directory_entry& directory_iterator::operator*() const {
    return impl_->dir_entry_;
}

MARS_FILESYSTEM_API const directory_entry* directory_iterator::operator->() const {
    return &impl_->dir_entry_;
}


};
};