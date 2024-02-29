//  boost/filesystem/directory.hpp  ---------------------------------------------------//

//  Copyright Beman Dawes 2002-2009
//  Copyright Jan Langer 2002
//  Copyright Dietmar Kuehl 2001
//  Copyright Vladimir Prus 2002
//  Copyright Andrey Semashev 2019, 2022

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

//--------------------------------------------------------------------------------------//

#ifndef BOOST_FILESYSTEM_DIRECTORY_HPP
#define BOOST_FILESYSTEM_DIRECTORY_HPP

#include <boost/filesystem/config.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/file_status.hpp>
#include <boost/filesystem/detail/path_traits.hpp>

#include <cstddef>
#include <string>
#include <vector>

#include <boost/assert.hpp>
#include <boost/core/scoped_enum.hpp>
#include <boost/detail/bitmask.hpp>
#include <boost/system/error_code.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_categories.hpp>

#include <boost/filesystem/detail/header.hpp> // must be the last #include

//--------------------------------------------------------------------------------------//

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace filesystem {

class directory_iterator;

namespace detail {

struct directory_iterator_params;

BOOST_FILESYSTEM_DECL void directory_iterator_construct(directory_iterator& it, path const& p, unsigned int opts, directory_iterator_params* params, system::error_code* ec);
BOOST_FILESYSTEM_DECL void directory_iterator_increment(directory_iterator& it, system::error_code* ec);

} // namespace detail

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                 directory_entry                                      //
//                                                                                      //
//--------------------------------------------------------------------------------------//

//  GCC has a problem with a member function named path within a namespace or
//  sub-namespace that also has a class named path. The workaround is to always
//  fully qualify the name path when it refers to the class name.

class directory_entry
{
    friend BOOST_FILESYSTEM_DECL void detail::directory_iterator_construct(directory_iterator& it, path const& p, unsigned int opts, detail::directory_iterator_params* params, system::error_code* ec);
    friend BOOST_FILESYSTEM_DECL void detail::directory_iterator_increment(directory_iterator& it, system::error_code* ec);

public:
    typedef mars_boost::filesystem::path::value_type value_type; // enables class path ctor taking directory_entry

    directory_entry() BOOST_NOEXCEPT {}

    explicit directory_entry(mars_boost::filesystem::path const& p);

#if BOOST_FILESYSTEM_VERSION >= 4
    directory_entry(mars_boost::filesystem::path const& p, system::error_code& ec) :
        m_path(p)
    {
        refresh_impl(&ec);
        if (ec)
            m_path.clear();
    }
#else
    directory_entry(mars_boost::filesystem::path const& p, file_status st, file_status symlink_st = file_status()) :
        m_path(p), m_status(st), m_symlink_status(symlink_st)
    {
    }
#endif

    directory_entry(directory_entry const& rhs) :
        m_path(rhs.m_path), m_status(rhs.m_status), m_symlink_status(rhs.m_symlink_status)
    {
    }

    directory_entry& operator=(directory_entry const& rhs)
    {
        m_path = rhs.m_path;
        m_status = rhs.m_status;
        m_symlink_status = rhs.m_symlink_status;
        return *this;
    }

    //  As of October 2015 the interaction between noexcept and =default is so troublesome
    //  for VC++, GCC, and probably other compilers, that =default is not used with noexcept
    //  functions. GCC is not even consistent for the same release on different platforms.

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    directory_entry(directory_entry&& rhs) BOOST_NOEXCEPT :
        m_path(static_cast< mars_boost::filesystem::path&& >(rhs.m_path)),
        m_status(static_cast< file_status&& >(rhs.m_status)),
        m_symlink_status(static_cast< file_status&& >(rhs.m_symlink_status))
    {
    }

    directory_entry& operator=(directory_entry&& rhs) BOOST_NOEXCEPT
    {
        m_path = static_cast< mars_boost::filesystem::path&& >(rhs.m_path);
        m_status = static_cast< file_status&& >(rhs.m_status);
        m_symlink_status = static_cast< file_status&& >(rhs.m_symlink_status);
        return *this;
    }

    void assign(mars_boost::filesystem::path&& p);

#if BOOST_FILESYSTEM_VERSION >= 4
    void assign(mars_boost::filesystem::path&& p, system::error_code& ec)
    {
        m_path = static_cast< mars_boost::filesystem::path&& >(p);
        refresh_impl(&ec);
    }
#else
    void assign(mars_boost::filesystem::path&& p, file_status st, file_status symlink_st = file_status())
    {
        assign_with_status(static_cast< mars_boost::filesystem::path&& >(p), st, symlink_st);
    }
#endif
#endif // !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

    void assign(mars_boost::filesystem::path const& p);

#if BOOST_FILESYSTEM_VERSION >= 4
    void assign(mars_boost::filesystem::path const& p, system::error_code& ec)
    {
        m_path = p;
        refresh_impl(&ec);
    }
#else
    void assign(mars_boost::filesystem::path const& p, file_status st, file_status symlink_st = file_status())
    {
        assign_with_status(p, st, symlink_st);
    }
#endif

    void replace_filename(mars_boost::filesystem::path const& p);

#if BOOST_FILESYSTEM_VERSION >= 4
    void replace_filename(mars_boost::filesystem::path const& p, system::error_code& ec)
    {
        m_path.replace_filename(p);
        refresh_impl(&ec);
    }
#else
    void replace_filename(mars_boost::filesystem::path const& p, file_status st, file_status symlink_st = file_status())
    {
        replace_filename_with_status(p, st, symlink_st);
    }

    BOOST_FILESYSTEM_DETAIL_DEPRECATED("Use directory_entry::replace_filename() instead")
    void replace_leaf(mars_boost::filesystem::path const& p, file_status st, file_status symlink_st)
    {
        replace_filename_with_status(p, st, symlink_st);
    }
#endif

    mars_boost::filesystem::path const& path() const BOOST_NOEXCEPT { return m_path; }
    operator mars_boost::filesystem::path const&() const BOOST_NOEXCEPT { return m_path; }

    void refresh() { refresh_impl(); }
    void refresh(system::error_code& ec) BOOST_NOEXCEPT { refresh_impl(&ec); }

    file_status status() const
    {
        if (!filesystem::status_known(m_status))
            refresh_impl();
        return m_status;
    }

    file_status status(system::error_code& ec) const BOOST_NOEXCEPT
    {
        ec.clear();

        if (!filesystem::status_known(m_status))
            refresh_impl(&ec);
        return m_status;
    }

    file_status symlink_status() const
    {
        if (!filesystem::status_known(m_symlink_status))
            refresh_impl();
        return m_symlink_status;
    }

    file_status symlink_status(system::error_code& ec) const BOOST_NOEXCEPT
    {
        ec.clear();

        if (!filesystem::status_known(m_symlink_status))
            refresh_impl(&ec);
        return m_symlink_status;
    }

    filesystem::file_type file_type() const
    {
        if (!filesystem::type_present(m_status))
            refresh_impl();
        return m_status.type();
    }

    filesystem::file_type file_type(system::error_code& ec) const BOOST_NOEXCEPT
    {
        ec.clear();

        if (!filesystem::type_present(m_status))
            refresh_impl(&ec);
        return m_status.type();
    }

    filesystem::file_type symlink_file_type() const
    {
        if (!filesystem::type_present(m_symlink_status))
            refresh_impl();
        return m_symlink_status.type();
    }

    filesystem::file_type symlink_file_type(system::error_code& ec) const BOOST_NOEXCEPT
    {
        ec.clear();

        if (!filesystem::type_present(m_symlink_status))
            refresh_impl(&ec);
        return m_symlink_status.type();
    }

    bool exists() const
    {
        filesystem::file_type ft = this->file_type();
        return ft != filesystem::status_error && ft != filesystem::file_not_found;
    }

    bool exists(system::error_code& ec) const BOOST_NOEXCEPT
    {
        filesystem::file_type ft = this->file_type(ec);
        return ft != filesystem::status_error && ft != filesystem::file_not_found;
    }

    bool is_regular_file() const
    {
        return this->file_type() == filesystem::regular_file;
    }

    bool is_regular_file(system::error_code& ec) const BOOST_NOEXCEPT
    {
        return this->file_type(ec) == filesystem::regular_file;
    }

    bool is_directory() const
    {
        return this->file_type() == filesystem::directory_file;
    }

    bool is_directory(system::error_code& ec) const BOOST_NOEXCEPT
    {
        return this->file_type(ec) == filesystem::directory_file;
    }

    bool is_symlink() const
    {
        return this->symlink_file_type() == filesystem::symlink_file;
    }

    bool is_symlink(system::error_code& ec) const BOOST_NOEXCEPT
    {
        return this->symlink_file_type(ec) == filesystem::symlink_file;
    }

    bool is_block_file() const
    {
        return this->file_type() == filesystem::block_file;
    }

    bool is_block_file(system::error_code& ec) const BOOST_NOEXCEPT
    {
        return this->file_type(ec) == filesystem::block_file;
    }

    bool is_character_file() const
    {
        return this->file_type() == filesystem::character_file;
    }

    bool is_character_file(system::error_code& ec) const BOOST_NOEXCEPT
    {
        return this->file_type(ec) == filesystem::character_file;
    }

    bool is_fifo() const
    {
        return this->file_type() == filesystem::fifo_file;
    }

    bool is_fifo(system::error_code& ec) const BOOST_NOEXCEPT
    {
        return this->file_type(ec) == filesystem::fifo_file;
    }

    bool is_socket() const
    {
        return this->file_type() == filesystem::socket_file;
    }

    bool is_socket(system::error_code& ec) const BOOST_NOEXCEPT
    {
        return this->file_type(ec) == filesystem::socket_file;
    }

    bool is_reparse_file() const
    {
        return this->symlink_file_type() == filesystem::reparse_file;
    }

    bool is_reparse_file(system::error_code& ec) const BOOST_NOEXCEPT
    {
        return this->symlink_file_type(ec) == filesystem::reparse_file;
    }

    bool is_other() const
    {
        filesystem::file_type ft = this->file_type();
        return ft != filesystem::status_error && ft != filesystem::file_not_found &&
            ft != filesystem::regular_file && ft != filesystem::directory_file;
    }

    bool is_other(system::error_code& ec) const BOOST_NOEXCEPT
    {
        filesystem::file_type ft = this->file_type(ec);
        return ft != filesystem::status_error && ft != filesystem::file_not_found &&
            ft != filesystem::regular_file && ft != filesystem::directory_file;
    }

    bool operator==(directory_entry const& rhs) const { return m_path == rhs.m_path; }
    bool operator!=(directory_entry const& rhs) const { return m_path != rhs.m_path; }
    bool operator<(directory_entry const& rhs) const { return m_path < rhs.m_path; }
    bool operator<=(directory_entry const& rhs) const { return m_path <= rhs.m_path; }
    bool operator>(directory_entry const& rhs) const { return m_path > rhs.m_path; }
    bool operator>=(directory_entry const& rhs) const { return m_path >= rhs.m_path; }

private:
    BOOST_FILESYSTEM_DECL void refresh_impl(system::error_code* ec = NULL) const;

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    void assign_with_status(mars_boost::filesystem::path&& p, file_status st, file_status symlink_st)
    {
        m_path = static_cast< mars_boost::filesystem::path&& >(p);
        m_status = static_cast< file_status&& >(st);
        m_symlink_status = static_cast< file_status&& >(symlink_st);
    }
#endif

    void assign_with_status(mars_boost::filesystem::path const& p, file_status st, file_status symlink_st)
    {
        m_path = p;
#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
        m_status = static_cast< file_status&& >(st);
        m_symlink_status = static_cast< file_status&& >(symlink_st);
#else
        m_status = st;
        m_symlink_status = symlink_st;
#endif
    }

    void replace_filename_with_status(mars_boost::filesystem::path const& p, file_status st, file_status symlink_st)
    {
        m_path.replace_filename(p);
#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
        m_status = static_cast< file_status&& >(st);
        m_symlink_status = static_cast< file_status&& >(symlink_st);
#else
        m_status = st;
        m_symlink_status = symlink_st;
#endif
    }

private:
    mars_boost::filesystem::path m_path;
    mutable file_status m_status;         // stat()-like
    mutable file_status m_symlink_status; // lstat()-like
};

#if !defined(BOOST_FILESYSTEM_SOURCE)

inline directory_entry::directory_entry(mars_boost::filesystem::path const& p) :
    m_path(p)
{
#if BOOST_FILESYSTEM_VERSION >= 4
    refresh_impl();
#endif
}

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
inline void directory_entry::assign(mars_boost::filesystem::path&& p)
{
    m_path = static_cast< mars_boost::filesystem::path&& >(p);
#if BOOST_FILESYSTEM_VERSION >= 4
    refresh_impl();
#else
    m_status = file_status();
    m_symlink_status = file_status();
#endif
}
#endif // !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

inline void directory_entry::assign(mars_boost::filesystem::path const& p)
{
    m_path = p;
#if BOOST_FILESYSTEM_VERSION >= 4
    refresh_impl();
#else
    m_status = file_status();
    m_symlink_status = file_status();
#endif
}

inline void directory_entry::replace_filename(mars_boost::filesystem::path const& p)
{
    m_path.replace_filename(p);
#if BOOST_FILESYSTEM_VERSION >= 4
    refresh_impl();
#else
    m_status = file_status();
    m_symlink_status = file_status();
#endif
}

#endif // !defined(BOOST_FILESYSTEM_SOURCE)

namespace detail {
namespace path_traits {

// Dispatch function for integration with path class
template< typename Callback >
BOOST_FORCEINLINE typename Callback::result_type dispatch(directory_entry const& de, Callback cb, const codecvt_type* cvt, directory_entry_tag)
{
    mars_boost::filesystem::path::string_type const& source = de.path().native();
    return cb(source.data(), source.data() + source.size(), cvt);
}

} // namespace path_traits
} // namespace detail

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                            directory_entry overloads                                 //
//                                                                                      //
//--------------------------------------------------------------------------------------//

//  Without these functions, calling (for example) 'is_directory' with a 'directory_entry' results in:
//  - a conversion to 'path' using 'operator mars_boost::filesystem::path const&()',
//  - then a call to 'is_directory(path const& p)' which recomputes the status with 'detail::status(p)'.
//
//  These functions avoid a costly recomputation of the status if one calls 'is_directory(e)' instead of 'is_directory(e.status())'

inline file_status status(directory_entry const& e)
{
    return e.status();
}

inline file_status status(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.status(ec);
}

inline file_status symlink_status(directory_entry const& e)
{
    return e.symlink_status();
}

inline file_status symlink_status(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.symlink_status(ec);
}

inline bool type_present(directory_entry const& e)
{
    return e.file_type() != filesystem::status_error;
}

inline bool type_present(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.file_type(ec) != filesystem::status_error;
}

inline bool status_known(directory_entry const& e)
{
    return filesystem::status_known(e.status());
}

inline bool status_known(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return filesystem::status_known(e.status(ec));
}

inline bool exists(directory_entry const& e)
{
    return e.exists();
}

inline bool exists(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.exists(ec);
}

inline bool is_regular_file(directory_entry const& e)
{
    return e.is_regular_file();
}

inline bool is_regular_file(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.is_regular_file(ec);
}

inline bool is_directory(directory_entry const& e)
{
    return e.is_directory();
}

inline bool is_directory(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.is_directory(ec);
}

inline bool is_symlink(directory_entry const& e)
{
    return e.is_symlink();
}

inline bool is_symlink(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.is_symlink(ec);
}

inline bool is_block_file(directory_entry const& e)
{
    return e.is_block_file();
}

inline bool is_block_file(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.is_block_file(ec);
}

inline bool is_character_file(directory_entry const& e)
{
    return e.is_character_file();
}

inline bool is_character_file(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.is_character_file(ec);
}

inline bool is_fifo(directory_entry const& e)
{
    return e.is_fifo();
}

inline bool is_fifo(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.is_fifo(ec);
}

inline bool is_socket(directory_entry const& e)
{
    return e.is_socket();
}

inline bool is_socket(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.is_socket(ec);
}

inline bool is_reparse_file(directory_entry const& e)
{
    return e.is_reparse_file();
}

inline bool is_reparse_file(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.is_reparse_file(ec);
}

inline bool is_other(directory_entry const& e)
{
    return e.is_other();
}

inline bool is_other(directory_entry const& e, system::error_code& ec) BOOST_NOEXCEPT
{
    return e.is_other(ec);
}

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
BOOST_FILESYSTEM_DETAIL_DEPRECATED("Use is_regular_file() instead")
inline bool is_regular(directory_entry const& e)
{
    return filesystem::is_regular_file(e);
}
#endif

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                            directory_iterator helpers                                //
//                                                                                      //
//--------------------------------------------------------------------------------------//

BOOST_SCOPED_ENUM_UT_DECLARE_BEGIN(directory_options, unsigned int)
{
    none = 0u,
    skip_permission_denied = 1u,         // if a directory cannot be opened because of insufficient permissions, pretend that the directory is empty
    follow_directory_symlink = 1u << 1u, // recursive_directory_iterator: follow directory symlinks
    skip_dangling_symlinks = 1u << 2u,   // non-standard extension for recursive_directory_iterator: don't follow dangling directory symlinks,
    pop_on_error = 1u << 3u,             // non-standard extension for recursive_directory_iterator: instead of producing an end iterator on errors,
                                         // repeatedly invoke pop() until it succeeds or the iterator becomes equal to end iterator
    _detail_no_follow = 1u << 4u,        // internal use only
    _detail_no_push = 1u << 5u           // internal use only
}
BOOST_SCOPED_ENUM_DECLARE_END(directory_options)

BOOST_BITMASK(BOOST_SCOPED_ENUM_NATIVE(directory_options))

namespace detail {

struct dir_itr_imp :
    public mars_boost::intrusive_ref_counter< dir_itr_imp >
{
#ifdef BOOST_WINDOWS_API
    bool close_handle;
    unsigned char extra_data_format;
    std::size_t current_offset;
#endif
    directory_entry dir_entry;
    void* handle;

    dir_itr_imp() BOOST_NOEXCEPT :
#ifdef BOOST_WINDOWS_API
        close_handle(false),
        extra_data_format(0u),
        current_offset(0u),
#endif
        handle(NULL)
    {
    }
    BOOST_FILESYSTEM_DECL ~dir_itr_imp() BOOST_NOEXCEPT;

    BOOST_FILESYSTEM_DECL static void* operator new(std::size_t class_size, std::size_t extra_size) BOOST_NOEXCEPT;
    BOOST_FILESYSTEM_DECL static void operator delete(void* p, std::size_t extra_size) BOOST_NOEXCEPT;
    BOOST_FILESYSTEM_DECL static void operator delete(void* p) BOOST_NOEXCEPT;
};

} // namespace detail

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                directory_iterator                                    //
//                                                                                      //
//--------------------------------------------------------------------------------------//

class directory_iterator :
    public mars_boost::iterator_facade<
        directory_iterator,
        directory_entry,
        mars_boost::single_pass_traversal_tag
    >
{
    friend class mars_boost::iterator_core_access;

    friend BOOST_FILESYSTEM_DECL void detail::directory_iterator_construct(directory_iterator& it, path const& p, unsigned int opts, detail::directory_iterator_params* params, system::error_code* ec);
    friend BOOST_FILESYSTEM_DECL void detail::directory_iterator_increment(directory_iterator& it, system::error_code* ec);

public:
    directory_iterator() BOOST_NOEXCEPT {} // creates the "end" iterator

    // iterator_facade derived classes don't seem to like implementations in
    // separate translation unit dll's, so forward to detail functions
    explicit directory_iterator(path const& p, BOOST_SCOPED_ENUM_NATIVE(directory_options) opts = directory_options::none)
    {
        detail::directory_iterator_construct(*this, p, static_cast< unsigned int >(opts), NULL, NULL);
    }

    directory_iterator(path const& p, system::error_code& ec) BOOST_NOEXCEPT
    {
        detail::directory_iterator_construct(*this, p, static_cast< unsigned int >(directory_options::none), NULL, &ec);
    }

    directory_iterator(path const& p, BOOST_SCOPED_ENUM_NATIVE(directory_options) opts, system::error_code& ec) BOOST_NOEXCEPT
    {
        detail::directory_iterator_construct(*this, p, static_cast< unsigned int >(opts), NULL, &ec);
    }

    BOOST_DEFAULTED_FUNCTION(directory_iterator(directory_iterator const& that), : m_imp(that.m_imp) {})
    BOOST_DEFAULTED_FUNCTION(directory_iterator& operator=(directory_iterator const& that), { m_imp = that.m_imp; return *this; })

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    directory_iterator(directory_iterator&& that) BOOST_NOEXCEPT :
        m_imp(static_cast< mars_boost::intrusive_ptr< detail::dir_itr_imp >&& >(that.m_imp))
    {
    }

    directory_iterator& operator=(directory_iterator&& that) BOOST_NOEXCEPT
    {
        m_imp = static_cast< mars_boost::intrusive_ptr< detail::dir_itr_imp >&& >(that.m_imp);
        return *this;
    }
#endif // !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

    directory_iterator& increment(system::error_code& ec) BOOST_NOEXCEPT
    {
        detail::directory_iterator_increment(*this, &ec);
        return *this;
    }

private:
    mars_boost::iterator_facade<
        directory_iterator,
        directory_entry,
        mars_boost::single_pass_traversal_tag
    >::reference dereference() const
    {
        BOOST_ASSERT_MSG(!is_end(), "attempt to dereference end directory iterator");
        return m_imp->dir_entry;
    }

    void increment() { detail::directory_iterator_increment(*this, NULL); }

    bool equal(directory_iterator const& rhs) const BOOST_NOEXCEPT
    {
        return m_imp == rhs.m_imp || (is_end() && rhs.is_end());
    }

    bool is_end() const BOOST_NOEXCEPT
    {
        // Note: The check for handle is needed because the iterator can be copied and the copy
        // can be incremented to end while the original iterator still refers to the same dir_itr_imp.
        return !m_imp || !m_imp->handle;
    }

private:
    // intrusive_ptr provides the shallow-copy semantics required for single pass iterators
    // (i.e. InputIterators). The end iterator is indicated by is_end().
    mars_boost::intrusive_ptr< detail::dir_itr_imp > m_imp;
};

//  enable directory_iterator C++11 range-based for statement use  --------------------//

// begin() and end() are only used by a range-based for statement in the context of
// auto - thus the top-level const is stripped - so returning const is harmless and
// emphasizes begin() is just a pass through.
inline directory_iterator const& begin(directory_iterator const& iter) BOOST_NOEXCEPT
{
    return iter;
}

inline directory_iterator end(directory_iterator const&) BOOST_NOEXCEPT
{
    return directory_iterator();
}

// enable C++14 generic accessors for range const iterators
inline directory_iterator const& cbegin(directory_iterator const& iter) BOOST_NOEXCEPT
{
    return iter;
}

inline directory_iterator cend(directory_iterator const&) BOOST_NOEXCEPT
{
    return directory_iterator();
}

//  enable directory_iterator BOOST_FOREACH  -----------------------------------------//

inline directory_iterator& range_begin(directory_iterator& iter) BOOST_NOEXCEPT
{
    return iter;
}

inline directory_iterator range_begin(directory_iterator const& iter) BOOST_NOEXCEPT
{
    return iter;
}

inline directory_iterator range_end(directory_iterator&) BOOST_NOEXCEPT
{
    return directory_iterator();
}

inline directory_iterator range_end(directory_iterator const&) BOOST_NOEXCEPT
{
    return directory_iterator();
}

} // namespace filesystem

//  namespace mars_boost template specializations
template< typename C, typename Enabler >
struct range_mutable_iterator;

template<>
struct range_mutable_iterator< mars_boost::filesystem::directory_iterator, void >
{
    typedef mars_boost::filesystem::directory_iterator type;
};

template< typename C, typename Enabler >
struct range_const_iterator;

template<>
struct range_const_iterator< mars_boost::filesystem::directory_iterator, void >
{
    typedef mars_boost::filesystem::directory_iterator type;
};

namespace filesystem {

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                      recursive_directory_iterator helpers                            //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#if !defined(BOOST_FILESYSTEM_NO_DEPRECATED)
// Deprecated enum, use directory_options instead
BOOST_SCOPED_ENUM_UT_DECLARE_BEGIN(symlink_option, unsigned int)
{
    none = static_cast< unsigned int >(directory_options::none),
    no_recurse = none,                                                                  // don't follow directory symlinks (default behavior)
    recurse = static_cast< unsigned int >(directory_options::follow_directory_symlink), // follow directory symlinks
    _detail_no_push = static_cast< unsigned int >(directory_options::_detail_no_push)   // internal use only
}
BOOST_SCOPED_ENUM_DECLARE_END(symlink_option)

BOOST_BITMASK(BOOST_SCOPED_ENUM_NATIVE(symlink_option))
#endif // BOOST_FILESYSTEM_NO_DEPRECATED

class recursive_directory_iterator;

namespace detail {

struct recur_dir_itr_imp :
    public mars_boost::intrusive_ref_counter< recur_dir_itr_imp >
{
    typedef directory_iterator element_type;
    std::vector< element_type > m_stack;
    // directory_options values, declared as unsigned int for ABI compatibility
    unsigned int m_options;

    explicit recur_dir_itr_imp(unsigned int opts) BOOST_NOEXCEPT : m_options(opts) {}
};

BOOST_FILESYSTEM_DECL void recursive_directory_iterator_construct(recursive_directory_iterator& it, path const& dir_path, unsigned int opts, system::error_code* ec);
BOOST_FILESYSTEM_DECL void recursive_directory_iterator_increment(recursive_directory_iterator& it, system::error_code* ec);
BOOST_FILESYSTEM_DECL void recursive_directory_iterator_pop(recursive_directory_iterator& it, system::error_code* ec);

} // namespace detail

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                           recursive_directory_iterator                               //
//                                                                                      //
//--------------------------------------------------------------------------------------//

class recursive_directory_iterator :
    public mars_boost::iterator_facade<
        recursive_directory_iterator,
        directory_entry,
        mars_boost::single_pass_traversal_tag
    >
{
    friend class mars_boost::iterator_core_access;

    friend BOOST_FILESYSTEM_DECL void detail::recursive_directory_iterator_construct(recursive_directory_iterator& it, path const& dir_path, unsigned int opts, system::error_code* ec);
    friend BOOST_FILESYSTEM_DECL void detail::recursive_directory_iterator_increment(recursive_directory_iterator& it, system::error_code* ec);
    friend BOOST_FILESYSTEM_DECL void detail::recursive_directory_iterator_pop(recursive_directory_iterator& it, system::error_code* ec);

public:
    recursive_directory_iterator() BOOST_NOEXCEPT {} // creates the "end" iterator

    explicit recursive_directory_iterator(path const& dir_path)
    {
        detail::recursive_directory_iterator_construct(*this, dir_path, static_cast< unsigned int >(directory_options::none), NULL);
    }

    recursive_directory_iterator(path const& dir_path, system::error_code& ec)
    {
        detail::recursive_directory_iterator_construct(*this, dir_path, static_cast< unsigned int >(directory_options::none), &ec);
    }

    recursive_directory_iterator(path const& dir_path, BOOST_SCOPED_ENUM_NATIVE(directory_options) opts)
    {
        detail::recursive_directory_iterator_construct(*this, dir_path, static_cast< unsigned int >(opts), NULL);
    }

    recursive_directory_iterator(path const& dir_path, BOOST_SCOPED_ENUM_NATIVE(directory_options) opts, system::error_code& ec)
    {
        detail::recursive_directory_iterator_construct(*this, dir_path, static_cast< unsigned int >(opts), &ec);
    }

#if !defined(BOOST_FILESYSTEM_NO_DEPRECATED)
    // Deprecated constructors
    BOOST_FILESYSTEM_DETAIL_DEPRECATED("Use directory_options instead of symlink_option")
    recursive_directory_iterator(path const& dir_path, BOOST_SCOPED_ENUM_NATIVE(symlink_option) opts)
    {
        detail::recursive_directory_iterator_construct(*this, dir_path, static_cast< unsigned int >(opts), NULL);
    }

    BOOST_FILESYSTEM_DETAIL_DEPRECATED("Use directory_options instead of symlink_option")
    recursive_directory_iterator(path const& dir_path, BOOST_SCOPED_ENUM_NATIVE(symlink_option) opts, system::error_code& ec) BOOST_NOEXCEPT
    {
        detail::recursive_directory_iterator_construct(*this, dir_path, static_cast< unsigned int >(opts), &ec);
    }
#endif // BOOST_FILESYSTEM_NO_DEPRECATED

    BOOST_DEFAULTED_FUNCTION(recursive_directory_iterator(recursive_directory_iterator const& that), : m_imp(that.m_imp) {})
    BOOST_DEFAULTED_FUNCTION(recursive_directory_iterator& operator=(recursive_directory_iterator const& that), { m_imp = that.m_imp; return *this; })

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    recursive_directory_iterator(recursive_directory_iterator&& that) BOOST_NOEXCEPT :
        m_imp(static_cast< mars_boost::intrusive_ptr< detail::recur_dir_itr_imp >&& >(that.m_imp))
    {
    }

    recursive_directory_iterator& operator=(recursive_directory_iterator&& that) BOOST_NOEXCEPT
    {
        m_imp = static_cast< mars_boost::intrusive_ptr< detail::recur_dir_itr_imp >&& >(that.m_imp);
        return *this;
    }
#endif // !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

    recursive_directory_iterator& increment(system::error_code& ec) BOOST_NOEXCEPT
    {
        detail::recursive_directory_iterator_increment(*this, &ec);
        return *this;
    }

    int depth() const BOOST_NOEXCEPT
    {
        BOOST_ASSERT_MSG(!is_end(), "depth() on end recursive_directory_iterator");
        return static_cast< int >(m_imp->m_stack.size() - 1u);
    }

    bool recursion_pending() const BOOST_NOEXCEPT
    {
        BOOST_ASSERT_MSG(!is_end(), "recursion_pending() on end recursive_directory_iterator");
        return (m_imp->m_options & static_cast< unsigned int >(directory_options::_detail_no_push)) == 0u;
    }

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
    BOOST_FILESYSTEM_DETAIL_DEPRECATED("Use recursive_directory_iterator::depth() instead")
    int level() const BOOST_NOEXCEPT
    {
        return depth();
    }
    bool no_push_pending() const BOOST_NOEXCEPT { return !recursion_pending(); }
    bool no_push_request() const BOOST_NOEXCEPT { return !recursion_pending(); }
#endif

    void pop()
    {
        detail::recursive_directory_iterator_pop(*this, NULL);
    }

    void pop(system::error_code& ec) BOOST_NOEXCEPT
    {
        detail::recursive_directory_iterator_pop(*this, &ec);
    }

    void disable_recursion_pending(bool value = true) BOOST_NOEXCEPT
    {
        BOOST_ASSERT_MSG(!is_end(), "disable_recursion_pending() on end recursive_directory_iterator");
        if (value)
            m_imp->m_options |= static_cast< unsigned int >(directory_options::_detail_no_push);
        else
            m_imp->m_options &= ~static_cast< unsigned int >(directory_options::_detail_no_push);
    }

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
    BOOST_FILESYSTEM_DETAIL_DEPRECATED("Use recursive_directory_iterator::disable_recursion_pending() instead")
    void no_push(bool value = true) BOOST_NOEXCEPT
    {
        disable_recursion_pending(value);
    }
#endif

    file_status status() const
    {
        BOOST_ASSERT_MSG(!is_end(), "status() on end recursive_directory_iterator");
        return m_imp->m_stack.back()->status();
    }

    file_status symlink_status() const
    {
        BOOST_ASSERT_MSG(!is_end(), "symlink_status() on end recursive_directory_iterator");
        return m_imp->m_stack.back()->symlink_status();
    }

private:
    mars_boost::iterator_facade<
        recursive_directory_iterator,
        directory_entry,
        mars_boost::single_pass_traversal_tag
    >::reference dereference() const
    {
        BOOST_ASSERT_MSG(!is_end(), "dereference of end recursive_directory_iterator");
        return *m_imp->m_stack.back();
    }

    void increment() { detail::recursive_directory_iterator_increment(*this, NULL); }

    bool equal(recursive_directory_iterator const& rhs) const BOOST_NOEXCEPT
    {
        return m_imp == rhs.m_imp || (is_end() && rhs.is_end());
    }

    bool is_end() const BOOST_NOEXCEPT
    {
        // Note: The check for m_stack.empty() is needed because the iterator can be copied and the copy
        // can be incremented to end while the original iterator still refers to the same recur_dir_itr_imp.
        return !m_imp || m_imp->m_stack.empty();
    }

private:
    // intrusive_ptr provides the shallow-copy semantics required for single pass iterators
    // (i.e. InputIterators). The end iterator is indicated by is_end().
    mars_boost::intrusive_ptr< detail::recur_dir_itr_imp > m_imp;
};

#if !defined(BOOST_FILESYSTEM_NO_DEPRECATED)
BOOST_FILESYSTEM_DETAIL_DEPRECATED("Use recursive_directory_iterator instead")
typedef recursive_directory_iterator wrecursive_directory_iterator;
#endif

//  enable recursive directory iterator C++11 range-base for statement use  ----------//

// begin() and end() are only used by a range-based for statement in the context of
// auto - thus the top-level const is stripped - so returning const is harmless and
// emphasizes begin() is just a pass through.
inline recursive_directory_iterator const& begin(recursive_directory_iterator const& iter) BOOST_NOEXCEPT
{
    return iter;
}

inline recursive_directory_iterator end(recursive_directory_iterator const&) BOOST_NOEXCEPT
{
    return recursive_directory_iterator();
}

// enable C++14 generic accessors for range const iterators
inline recursive_directory_iterator const& cbegin(recursive_directory_iterator const& iter) BOOST_NOEXCEPT
{
    return iter;
}

inline recursive_directory_iterator cend(recursive_directory_iterator const&) BOOST_NOEXCEPT
{
    return recursive_directory_iterator();
}

//  enable recursive directory iterator BOOST_FOREACH  -------------------------------//

inline recursive_directory_iterator& range_begin(recursive_directory_iterator& iter) BOOST_NOEXCEPT
{
    return iter;
}

inline recursive_directory_iterator range_begin(recursive_directory_iterator const& iter) BOOST_NOEXCEPT
{
    return iter;
}

inline recursive_directory_iterator range_end(recursive_directory_iterator&) BOOST_NOEXCEPT
{
    return recursive_directory_iterator();
}

inline recursive_directory_iterator range_end(recursive_directory_iterator const&) BOOST_NOEXCEPT
{
    return recursive_directory_iterator();
}

} // namespace filesystem

//  namespace mars_boost template specializations
template<>
struct range_mutable_iterator< mars_boost::filesystem::recursive_directory_iterator, void >
{
    typedef mars_boost::filesystem::recursive_directory_iterator type;
};

template<>
struct range_const_iterator< mars_boost::filesystem::recursive_directory_iterator, void >
{
    typedef mars_boost::filesystem::recursive_directory_iterator type;
};

} // namespace mars_boost

#include <boost/filesystem/detail/footer.hpp>

#endif // BOOST_FILESYSTEM_DIRECTORY_HPP
