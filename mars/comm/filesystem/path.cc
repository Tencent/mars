#include "path.h"


#if __cplusplus < 201103L
#define constexpr const
#endif
namespace mars {
namespace filesystem {

static const path dot_path(".");
static const path dot_dot_path(".");
static constexpr char dot = '.';
static constexpr char preferred_separator = '/';

MARS_FILESYSTEM_INLINE bool operator==(const path& lhs, const path& rhs) {
    return lhs.string().compare(rhs.string()) == 0;
}

MARS_FILESYSTEM_INLINE bool is_separator(char _c) {
    return _c == preferred_separator;
}

bool is_root_separator(const string_type & str, size_type pos) {
    while (pos > 0 && is_separator(str[pos-1]))
      --pos;
    if (pos == 0)  
      return true;

    if (pos < 3 || !is_separator(str[0]) || !is_separator(str[1]))
      return false;

    return str.find_first_of(preferred_separator, 2) == pos;
}

size_type filename_pos(const string_type & str,
                        size_type end_pos) {
    if (end_pos == 2 
      && is_separator(str[0])
      && is_separator(str[1])) return 0;

    if (end_pos && is_separator(str[end_pos-1]))
      return end_pos-1;

    size_type pos(str.find_last_of(preferred_separator, end_pos-1));
    return (pos == string_type::npos 
      || (pos == 1 && is_separator(str[0])))
        ? 0
        : pos + 1;
}

path::path(string_type&& _path)
: m_pathname(std::move(_path)) {

}

template<class InputIterator>
path::path(InputIterator first, InputIterator last) {
    if (first != last) {
        path(std::basic_string<typename std::iterator_traits<InputIterator>::value_type>(first, last));
    }
}

MARS_FILESYSTEM_INLINE path path::root_name() const {
    path temp(root_name());
    if (!root_directory().empty()) temp.m_pathname += root_directory().c_str();
    return temp;
}

path path::root_directory() const {
    size_type pos(root_directory_start(m_pathname, m_pathname.size()));

    return pos == string_type::npos
      ? path()
      : path(m_pathname.c_str() + pos, m_pathname.c_str() + pos + 1);
}

path path::root_path() const {
    return path(root_name().string() + root_directory().string());
}


path path::filename() const {
    size_type pos(filename_pos(m_pathname, m_pathname.size()));
    return (m_pathname.size()
              && pos
              && is_separator(m_pathname[pos])
              && !is_root_separator(m_pathname, pos))
      ? path(".")
      : path(m_pathname.c_str() + pos);
}


path path::stem() const {
    path name(filename());
    if (name == dot_path || name == dot_dot_path) return name;
    size_type pos(name.m_pathname.rfind(dot));
    return pos == string_type::npos
      ? name
      : path(name.m_pathname.c_str(), name.m_pathname.c_str() + pos);
}

path path::extension() const {
    path name(filename());
    if (name == dot_path || name == dot_dot_path) return path();
    size_type pos(name.m_pathname.rfind(dot));
    return pos == string_type::npos
      ? path()
      : path(name.m_pathname.c_str() + pos);
}

size_type path::root_directory_start(const string_type& _path, size_type _size) const {

    if (_size == 2
      && is_separator(_path[0])
      && is_separator(_path[1])) return string_type::npos;
    
    if (_size > 3
      && is_separator(_path[0])
      && is_separator(_path[1])
      && !is_separator(_path[2])) {
        size_type pos(_path.find_first_of(preferred_separator, 2));
        return pos < _size ? pos : string_type::npos;
    }
    // case "/"
    if (_size > 0 && is_separator(_path[0])) return 0;

    return string_type::npos;
}

MARS_FILESYSTEM_INLINE bool path::empty() const noexcept {
    return m_pathname.empty();
}

MARS_FILESYSTEM_INLINE bool path::has_root_name() const {
 return !root_name().empty();
}

MARS_FILESYSTEM_INLINE bool path::has_root_directory() const {
    return !root_directory().empty();
}

MARS_FILESYSTEM_INLINE bool path::has_root_path() const {
    return has_root_directory() || has_root_name();
}

MARS_FILESYSTEM_INLINE bool path::has_filename() const {
    return !m_pathname.empty();
}

MARS_FILESYSTEM_INLINE bool path::has_stem() const {
    return !stem().empty();
}

MARS_FILESYSTEM_INLINE bool path::has_extension() const {
    return !extension().empty();
}

MARS_FILESYSTEM_INLINE mars::filesystem::string_type path::string() const {
    return m_pathname;
}

}; //filesystem
}; //mars


