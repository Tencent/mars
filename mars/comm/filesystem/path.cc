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

path::path() noexcept {}


path::path(const path& _p)
: pathname(_p.pathname) {
    
}

path::path(path&& _p) noexcept
    : pathname(std::move(_p.pathname)){}

path::path(string_type&& _path)
: pathname(std::move(_path)) {
}

path::path(string_type& _p) 
: pathname(_p) {
}

path::path(const string_type& _p) 
: pathname(_p){
}
path::path(char_type* _p)
: pathname(_p) {

}
path::path(const char_type* _p)
: pathname(_p) {

}

template<class InputIterator>
path::path(InputIterator first, InputIterator last)
:path(std::basic_string<typename std::iterator_traits<InputIterator>::value_type>(first, last)) {}

MARS_FILESYSTEM_INLINE path path::root_name() const {
    return path(pathname.substr(prefixLength_, root_name_length()));
}

path path::root_directory() const {
    size_type pos(root_directory_start(pathname, pathname.size()));

    return pos == string_type::npos
      ? path()
      : path(pathname.c_str() + pos, pathname.c_str() + pos + 1);
}

path path::root_path() const {
    return path(root_name().string() + root_directory().string());
}


path path::filename() const {
    size_type pos(filename_pos(pathname, pathname.size()));
    return (pathname.size()
              && pos
              && is_separator(pathname[pos])
              && !is_root_separator(pathname, pos))
      ? path(".")
      : path(pathname.c_str() + pos);
}


path path::stem() const {
    path name(filename());
    if (name == dot_path || name == dot_dot_path) return name;
    size_type pos(name.pathname.rfind(dot));
    return pos == string_type::npos
      ? name
      : path(name.pathname.c_str(), name.pathname.c_str() + pos);
}

path path::extension() const {
    path name(filename());
    if (name == dot_path || name == dot_dot_path) return path();
    size_type pos(name.pathname.rfind(dot));
    return pos == string_type::npos
      ? path()
      : path(name.pathname.c_str() + pos);
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

MARS_FILESYSTEM_INLINE path path::parent_path() const {
    auto rootPathLen = prefixLength_ + root_name_length() + (has_root_directory() ? 1 : 0);
    if (rootPathLen < pathname.length()) {
        if (empty()) {
            return path();
        }
        else {
            auto piter = end();
            auto iter = piter.decrement(pathname.end());
            if (iter > pathname.begin() + static_cast<long>(rootPathLen) && *iter != preferred_separator) {
                --iter;
            }
            return path(pathname.begin(), iter);
        }
    }
    else {
        return *this;
    }
}

MARS_FILESYSTEM_INLINE bool path::empty() const noexcept {
    return pathname.empty();
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
    return !pathname.empty();
}

MARS_FILESYSTEM_INLINE bool path::has_stem() const {
    return !stem().empty();
}

MARS_FILESYSTEM_INLINE bool path::has_extension() const {
    return !extension().empty();
}

MARS_FILESYSTEM_INLINE path& path::operator=(const path& _p) {
    pathname = _p.pathname;
    return *this;
}

MARS_FILESYSTEM_INLINE path& path::operator=(path&& _p) noexcept {
    pathname = std::move(_p.pathname);
    return *this;
}

MARS_FILESYSTEM_INLINE path& path::operator/=(const path& p) {
    if(p.empty())
      return *this;
    if (this == &p)  // self-append
    {
      path rhs(p);
      if (!is_separator(rhs.pathname[0]))
        m_append_separator_if_needed();
      pathname += rhs.pathname;
    }
    else
    {
      if (!is_separator(*p.pathname.begin()))
        m_append_separator_if_needed();
      pathname += p.pathname;
    }
    return *this;
}

size_type path::m_append_separator_if_needed() {
    if (!pathname.empty() && !is_separator(*(pathname.end()-1))) {
        size_type tmp(pathname.size());
        pathname += preferred_separator;
        return tmp;
    }
    return 0;
}

MARS_FILESYSTEM_INLINE void path::append_name(const value_type* name) {
    if (pathname.empty()) {
        this->operator/=(path(name));
    } else {
        if (pathname.back() != preferred_separator) {
            pathname.push_back(preferred_separator);
        }
        pathname += name;
    }
}

MARS_FILESYSTEM_INLINE size_type path::root_name_length() const noexcept {
    if (pathname.length() > prefixLength_ + 2 && pathname[prefixLength_] == preferred_separator && pathname[prefixLength_ + 1] == preferred_separator && pathname[prefixLength_ + 2] != preferred_separator && std::isprint(pathname[prefixLength_ + 2])) {
        size_type pos = pathname.find(preferred_separator, prefixLength_ + 3);
        if (pos == string_type::npos) {
            return pathname.length();
        }
        else {
            return pos;
        }
    }
    return 0;
}


template <class InputIterator>
MARS_FILESYSTEM_INLINE path& path::assign(InputIterator first, InputIterator last)
{
    pathname.assign(first, last);
    postprocess_path_with_format();
    return *this;
}

MARS_FILESYSTEM_INLINE void path::postprocess_path_with_format() {

    if (pathname.length() > prefixLength_ + 2 && pathname[prefixLength_] == preferred_separator && pathname[prefixLength_ + 1] == preferred_separator && pathname[prefixLength_ + 2] != preferred_separator) {
        string_type::iterator new_end = std::unique(pathname.begin() + static_cast<string_type::difference_type>(prefixLength_) + 2, pathname.end(), [](value_type lhs, value_type rhs) { return lhs == rhs && lhs == preferred_separator; });
        pathname.erase(new_end, pathname.end());
    }
    else {
        string_type::iterator new_end = std::unique(pathname.begin() + static_cast<string_type::difference_type>(prefixLength_), pathname.end(), [](value_type lhs, value_type rhs) { return lhs == rhs && lhs == preferred_separator; });
        pathname.erase(new_end, pathname.end());
    }
}


MARS_FILESYSTEM_INLINE bool operator==(const path& lhs, const path& rhs) noexcept {
    return lhs.string() == rhs.string();
}
MARS_FILESYSTEM_INLINE bool operator==(const path& lhs, const std::string& rhs) noexcept {
    return lhs.string() == rhs;
}

MARS_FILESYSTEM_INLINE bool operator!=(const path& lhs, const path& rhs) noexcept {
    return !(lhs.string() == rhs.string());
}



MARS_FILESYSTEM_INLINE path::iterator::iterator() {}

MARS_FILESYSTEM_INLINE path::iterator::iterator(const path& p, const string_type::const_iterator& pos)
    : _first(p.pathname.begin())
    , _last(p.pathname.end())
    , _prefix(_first + static_cast<string_type::difference_type>(p.prefixLength_))
    , _root(p.has_root_directory() ? _first + static_cast<string_type::difference_type>(p.prefixLength_ + p.root_name_length()) : _last)
    , _iter(pos)
{
    if(pos != _last) {
        updateCurrent();
    }
}

MARS_FILESYSTEM_INLINE string_type::const_iterator path::iterator::increment(const string_type::const_iterator& pos) const
{
    string_type::const_iterator i = pos;
    bool fromStart = i == _first || i == _prefix;
    if (i != _last) {
        if (fromStart && i == _first && _prefix > _first) {
            i = _prefix;
        }
        else if (*i++ == preferred_separator) {
            // we can only sit on a slash if it is a network name or a root
            if (i != _last && *i == preferred_separator) {
                if (fromStart && !(i + 1 != _last && *(i + 1) == preferred_separator)) {
                    // leadind double slashes detected, treat this and the
                    // following until a slash as one unit
                    i = std::find(++i, _last, preferred_separator);
                }
                else {
                    // skip redundant slashes
                    while (i != _last && *i == preferred_separator) {
                        ++i;
                    }
                }
            }
        }
        else {
            if (fromStart && i != _last && *i == ':') {
                ++i;
            }
            else {
                i = std::find(i, _last, preferred_separator);
            }
        }
    }
    return i;
}

MARS_FILESYSTEM_INLINE string_type::const_iterator path::iterator::decrement(const string_type::const_iterator& pos) const
{
    string_type::const_iterator i = pos;
    if (i != _first) {
        --i;
        // if this is now the root slash or the trailing slash, we are done,
        // else check for network name
        if (i != _root && (pos != _last || *i != preferred_separator)) {
            i = std::find(std::reverse_iterator<string_type::const_iterator>(i), std::reverse_iterator<string_type::const_iterator>(_first), preferred_separator).base();
            // Now we have to check if this is a network name
            if (i - _first == 2 && *_first == preferred_separator && *(_first + 1) == preferred_separator) {
                i -= 2;
            }
        }
    }
    return i;
}

MARS_FILESYSTEM_INLINE void path::iterator::updateCurrent()
{
    if ((_iter == _last) || (_iter != _first && _iter != _last && (*_iter == preferred_separator && _iter != _root) && (_iter + 1 == _last))) {
        _current.clear();
    }
    else {
        _current.assign(_iter, increment(_iter));
    }
}

MARS_FILESYSTEM_INLINE path::iterator& path::iterator::operator++()
{
    _iter = increment(_iter);
    while (_iter != _last &&                // we didn't reach the end
           _iter != _root &&                // this is not a root position
           *_iter == preferred_separator &&  // we are on a separator
           (_iter + 1) != _last             // the slash is not the last char
    ) {
        ++_iter;
    }
    updateCurrent();
    return *this;
}

MARS_FILESYSTEM_INLINE path::iterator path::iterator::operator++(int)
{
    path::iterator i{*this};
    ++(*this);
    return i;
}

MARS_FILESYSTEM_INLINE path::iterator& path::iterator::operator--()
{
    _iter = decrement(_iter);
    updateCurrent();
    return *this;
}

MARS_FILESYSTEM_INLINE path::iterator path::iterator::operator--(int)
{
    auto i = *this;
    --(*this);
    return i;
}

MARS_FILESYSTEM_INLINE bool path::iterator::operator==(const path::iterator& other) const
{
    return _iter == other._iter;
}

MARS_FILESYSTEM_INLINE bool path::iterator::operator!=(const path::iterator& other) const
{
    return _iter != other._iter;
}

MARS_FILESYSTEM_INLINE path::iterator::reference path::iterator::operator*() const
{
    return _current;
}

MARS_FILESYSTEM_INLINE path::iterator::pointer path::iterator::operator->() const
{
    return &_current;
}

MARS_FILESYSTEM_INLINE path::iterator path::begin() const
{
    return iterator(*this, pathname.begin());
}

MARS_FILESYSTEM_INLINE path::iterator path::end() const
{
    return iterator(*this, pathname.end());
}

}; //filesystem
}; //mars


