#include "operations.h"
#include "macros.h"

namespace mars {
namespace filesystem {

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

};
};