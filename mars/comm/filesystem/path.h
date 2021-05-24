#ifndef MARS_FILE_SYSTEM_PATH_H_
#define MARS_FILE_SYSTEM_PATH_H_

#include "macros.h"
#include <string>


namespace mars {
namespace filesystem {

typedef std::string::size_type size_type;
typedef std::string::value_type value_type;
typedef char char_type;
using string_type = std::string;
class MARS_FILESYSTEM_API path {

public:
    path() noexcept;
    path(const path& _p);
    path(path&& _p) noexcept;
    path(string_type&& _path);
    path(string_type& _p);
    path(char_type* _p);
    path(const char_type* _p);
    path(const string_type& _p);
    template<class InputIterator>
    path(InputIterator first, InputIterator last);
    ~path() = default;

    path& operator=(const path& _p);
    path& operator=(path&& _p) noexcept;
    path& operator/=(const path& p);

    path root_name() const;
    path root_directory() const;
    path root_path() const;
    path filename() const;
    path stem() const;
    path extension() const;
    path parent_path() const;

    bool empty() const noexcept;
    bool has_root_name() const;
    bool has_root_directory() const;
    bool has_root_path() const;
    bool has_filename() const;
    bool has_stem() const;
    bool has_extension() const;
    // bool is_absolute() const;
    // bool is_relative() const;
    class iterator;
    using const_iterator = iterator;
    iterator begin() const;
    iterator end() const;


    void append_name(const value_type* name);

    size_type root_directory_start(const string_type& _path, size_type _size) const;
    size_type m_append_separator_if_needed();
    size_type root_name_length() const noexcept;

    template <class InputIterator>
    path& assign(InputIterator first, InputIterator last);
    void postprocess_path_with_format();

    const char* c_str() const noexcept {return pathname.c_str();}
    MARS_FILESYSTEM_INLINE string_type string() const {return pathname;}
    void clear() noexcept {pathname.clear();}

public:
    string_type pathname;

    static const string_type::size_type prefixLength_{0};

};

bool operator==(const path& lhs, const path& rhs) noexcept;
bool operator==(const path& lhs, const std::string& rhs) noexcept;
bool operator!=(const path& lhs, const path& rhs) noexcept;



class MARS_FILESYSTEM_API path::iterator
{
public:
    using value_type = const path;
    using difference_type = std::ptrdiff_t;
    using pointer = const path*;
    using reference = const path&;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator();
    iterator(const path& p, const string_type::const_iterator& pos);
    iterator& operator++();
    iterator operator++(int);
    iterator& operator--();
    iterator operator--(int);
    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;
    reference operator*() const;
    pointer operator->() const;

private:
    friend class path;
    string_type::const_iterator increment(const string_type::const_iterator& pos) const;
    string_type::const_iterator decrement(const string_type::const_iterator& pos) const;
    void updateCurrent();
    string_type::const_iterator _first;
    string_type::const_iterator _last;
    string_type::const_iterator _prefix;
    string_type::const_iterator _root;
    string_type::const_iterator _iter;
    path _current;
};


}; //namespace filesystem
};  //namespace mars

#endif