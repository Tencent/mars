#ifndef MARS_FILESYSTEM_OPERATIONS_H_
#define MARS_FILESYSTEM_OPERATIONS_H_

#include <dirent.h>
#include <system_error>
#include <chrono>
#include "path.h"

namespace mars {
namespace filesystem {

using file_time_type = std::chrono::time_point<std::chrono::system_clock>;

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
    explicit file_status(mars::filesystem::file_type _ft, perms _prms = perms::unknown) noexcept;
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

    void refresh(std::error_code& _ec) noexcept;
    bool exists(std::error_code& _ec) const;
    bool is_block_file(std::error_code& _ec) const;
    bool is_character_file(std::error_code& _ec) const;
    bool is_directory(std::error_code& _ec) const;
    bool is_regular_file(std::error_code& _ec) const;
    uintmax_t file_size(std::error_code& _ec) const;
    file_time_type last_write_time(std::error_code& _ec) const;
    file_status status(std::error_code& _ec) const;

private:
    file_type status_file_type(std::error_code& _ec) const;
    path path_;
    file_status status_;
    file_status symlink_status_;
    uintmax_t file_size_ = static_cast<uintmax_t>(-1);
    uintmax_t hard_link_count_ = static_cast<uintmax_t>(-1);
    time_t last_write_time_ = 0;

};

class MARS_FILESYSTEM_API directory_iterator {
public:
    directory_iterator() noexcept;
    explicit directory_iterator(const path& _p);
    directory_iterator& operator++();
    class impl;
};

class MARS_FILESYSTEM_API directory_iterator::impl {
public:
    impl(const path& _p, directory_options _op) {

    }

public:
    path path_;
    directory_options options_;
    DIR* dir_;
    struct ::dirent* entry_;


};

};
};

#endif