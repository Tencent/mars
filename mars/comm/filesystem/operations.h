#ifndef MARS_FILESYSTEM_OPERATIONS_H_
#define MARS_FILESYSTEM_OPERATIONS_H_

#include <dirent.h>
#include <system_error>
#include <chrono>
#include <memory>

#include "macros.h"
#include "path.h"


namespace mars {
namespace filesystem {

using file_time_type = std::chrono::time_point<std::chrono::system_clock>;

class file_status;

struct space_info {
    uintmax_t capacity;
    uintmax_t free;
    uintmax_t available;
};

std::error_code make_system_error(int _err = 0);
bool is_directory(const path& _p);
bool is_regular_file(file_status s) ;
bool is_regular_file(const path& _p) ;
bool exists(const path& p) ;
bool exists(const path& p, std::error_code& ec) noexcept;
intmax_t file_size(const path& _p);
uintmax_t file_size(const path& _p, std::error_code& _ec) noexcept;
file_time_type last_write_time(const path& _p);
file_time_type last_write_time(const path& p, std::error_code& ec) noexcept;
bool remove(const path& p);
bool is_directory(file_status s);
uintmax_t remove_all(const path& p);
space_info space(const path& p);
bool create_directory(const path& p);
bool create_directory(const path& p, std::error_code& ec) noexcept;
bool create_directories(const path& p);
bool create_directories(const path& _p, std::error_code& _ec) noexcept;
bool remove(const path& p, std::error_code& ec) noexcept;
void resize_file(const path& p, uintmax_t size, std::error_code& ec) noexcept;
file_status status(const path& _p);
file_status status(const path& _p, std::error_code& _ec) noexcept;
void rename(const path& from, const path& to, std::error_code& ec) noexcept;

enum class directory_options  : uint16_t {
    none = 0,
    follow_directory_symlink = 1,
    skip_permission_denied = 2,
};

enum class file_type {
    none,
    not_found,
    regular,
    directory,
    symlink,
    block,
    character,
    fifo,
    socket,
    unknown,
};

enum class perms : uint16_t {
    none = 0,

    owner_read = 0400,
    owner_write = 0200,
    owner_exec = 0100,
    owner_all = 0700,

    group_read = 040,
    group_write = 020,
    group_exec = 010,
    group_all = 070,

    others_read = 04,
    others_write = 02,
    others_exec = 01,
    others_all = 07,

    all = 0777,
    set_uid = 04000,
    set_gid = 02000,
    sticky_bit = 01000,

    mask = 07777,
    unknown = 0xffff
};

class MARS_FILESYSTEM_API file_status {
public:
    file_status() noexcept;
    explicit file_status(mars::filesystem::file_type _ft, mars::filesystem::perms _prms = mars::filesystem::perms::unknown) noexcept;
    file_status(const file_status& _other) noexcept;
    file_status(file_status&& _other) noexcept;
    ~file_status() = default;
    file_status& operator=(const file_status& _rhs) noexcept;
    file_status& operator=(file_status&& _rhs) noexcept;

    void type(file_type _ft) noexcept;
    void permissions(perms _pm) noexcept;
    file_type type() const noexcept;
    perms permissions() const noexcept;
private:
    file_type type_;
    perms perms_;

};

class MARS_FILESYSTEM_API directory_entry {
public:
    directory_entry() noexcept = default;
    directory_entry(const path& _p, std::error_code& _ec);
    MARS_FILESYSTEM_INLINE ~directory_entry() = default;

    const filesystem::path& path() const noexcept;

    void refresh() noexcept;
    bool exists() const;
    bool is_block_file() const;
    bool is_character_file() const;
    bool is_directory() const;
    bool is_regular_file() const;
    uintmax_t file_size() const;
    file_time_type last_write_time() const;
    file_status status() const;
    bool is_symlink() const;

private:
    friend class directory_iterator;

    file_type status_file_type() const;
    mars::filesystem::path path_;
    file_status status_;
    file_status symlink_status_;
    uintmax_t file_size_ = static_cast<uintmax_t>(-1);
    uintmax_t hard_link_count_ = static_cast<uintmax_t>(-1);
    time_t last_write_time_ = 0;

};

class MARS_FILESYSTEM_API directory_iterator {
public:
    directory_iterator() noexcept;
    directory_iterator(const path& _p) noexcept;
    directory_iterator(const directory_iterator& _rhs);
    directory_iterator(const directory_iterator&& _rhs) noexcept;
    ~directory_iterator() {};
    directory_iterator& operator=(const directory_iterator &_rhs);
    directory_iterator& operator=(directory_iterator&& _rhs) noexcept;
    directory_iterator& operator++();
    bool operator==(const directory_iterator& _rhs) const;
    bool operator!=(const directory_iterator& _rhs) const;
    const directory_entry& operator*() const;
    const directory_entry* operator->() const;

private:
    class impl;
    std::shared_ptr<impl> impl_;
};


directory_iterator begin(directory_iterator iter) noexcept;
directory_iterator end(const directory_iterator&) noexcept;

class MARS_FILESYSTEM_API directory_iterator::impl {
public:
    impl(const path& _p, directory_options _op)
    : path_(_p)
    , options_(_op)
    , dir_(nullptr)
    , entry_(nullptr) {
        if (!path_.empty()) {
            dir_ = ::opendir(path_.string().c_str());
            if (!dir_) {
                auto error = errno;
                path_ = mars::filesystem::path();
                if ((error != EACCES && error != EPERM) || (static_cast<uint16_t>(options_) & static_cast<uint16_t>(directory_options::skip_permission_denied)) == static_cast<uint16_t>(directory_options::none)) {
                    ec_ = make_system_error();
                }
            } else {
                increment(ec_);
            }
        }
    }
    impl(const impl& _i) = delete;
    ~impl() {
        if (dir_) {
            ::closedir(dir_);
        }
    }

    void increment(std::error_code& ec)
    {
        if (dir_) {
            bool skip;
            do {
                skip = false;
                errno = 0;
                entry_ = ::readdir(dir_);
                if (entry_) {
                    dir_entry_.path_ = path_;
                    dir_entry_.path_.append_name(entry_->d_name);
                    copyToDirEntry();
                    if (ec && (ec.value() == EACCES || ec.value() == EPERM) && (static_cast<uint16_t>(options_) & 
                                static_cast<uint16_t>(mars::filesystem::directory_options::skip_permission_denied)) == static_cast<uint16_t>(mars::filesystem::directory_options::skip_permission_denied)) {
                        ec.clear();
                        skip = true;
                    }
                }
                else {
                    ::closedir(dir_);
                    dir_ = nullptr;
                    dir_entry_.path_.clear();
                    if (errno) {
                        ec = make_system_error();
                    }
                    break;
                }
            } while (skip || std::strcmp(entry_->d_name, ".") == 0 || std::strcmp(entry_->d_name, "..") == 0);
        }
    }

    void copyToDirEntry()
    {
        dir_entry_.symlink_status_.permissions(mars::filesystem::perms::unknown);
        switch(entry_->d_type) {
            case DT_BLK:  dir_entry_.symlink_status_.type(mars::filesystem::file_type::block); break;
            case DT_CHR:  dir_entry_.symlink_status_.type(mars::filesystem::file_type::character); break;
            case DT_DIR:  dir_entry_.symlink_status_.type(mars::filesystem::file_type::directory); break;
            case DT_FIFO: dir_entry_.symlink_status_.type(mars::filesystem::file_type::fifo); break;
            case DT_LNK:  dir_entry_.symlink_status_.type(mars::filesystem::file_type::symlink); break;
            case DT_REG:  dir_entry_.symlink_status_.type(mars::filesystem::file_type::regular); break;
            case DT_SOCK: dir_entry_.symlink_status_.type(mars::filesystem::file_type::socket); break;
            default:      dir_entry_.symlink_status_.type(mars::filesystem::file_type::unknown); break;
        }
        if (entry_->d_type != DT_LNK) {
            dir_entry_.status_ = dir_entry_.symlink_status_;
        }
        else {
            dir_entry_.status_.type(mars::filesystem::file_type::none);
            dir_entry_.status_.permissions(mars::filesystem::perms::unknown);
        }
        dir_entry_.file_size_ = static_cast<uintmax_t>(-1);
        dir_entry_.hard_link_count_ = static_cast<uintmax_t>(-1);
        dir_entry_.last_write_time_ = 0;
    }

public:
    path path_;
    directory_options options_;
    DIR* dir_;
    struct ::dirent* entry_;
    directory_entry dir_entry_;
    std::error_code ec_;

};

};
};

#endif