#ifndef MAPPED_FILE_H_
#define MAPPED_FILE_H_

#include <memory>
#include <iostream>

#include "path.h"

#define MARS_STATIC_CONSTANT(type, assignment) static const type assignment

struct na
{
    typedef na type;
    enum { value = 0 };
};

#define MARS_MPL_AUX_NA_PARAM(param) param = na

namespace mpl_{
   template <bool B> struct bool_;
   template <class I, I val> struct integral_c;
   struct integral_c_tag;
}

namespace mpl
{
    using mpl_::bool_;
    using mpl_::integral_c;
    using mpl_::integral_c_tag;
}


namespace mars {
namespace filesystem {
class mapped_file_source;
}}

namespace mars {
namespace filesystem {


typedef intmax_t stream_offset;
typedef int      file_handle;
// typedef unsigned int openmode;
// static const openmode app    = 0x01;
// static const openmode ate    = 0x02;
// static const openmode binary = 0x04;
// static const openmode in     = 0x08;
// static const openmode out    = 0x10;
// static const openmode trunc  = 0x20;


struct integral_c_tag {static const int value = 0; };

template< bool C_ > struct bool_
{
    static const bool value = C_;
    typedef integral_c_tag tag;
    typedef bool_ type;
    typedef bool value_type;
    operator bool() const { return this->value; }
};



struct any_tag {};
struct closable_tag : virtual any_tag {};
struct device_tag : virtual any_tag { };
struct filter_tag : virtual any_tag { };
struct input : virtual any_tag { };
struct output : virtual any_tag { };
struct one_head : virtual any_tag { };
struct source_tag : device_tag, input { };
struct direct_tag : virtual any_tag { };
struct random_access : virtual any_tag { }; 
struct input_seekable : virtual input, virtual random_access { };
struct output_seekable : virtual output, virtual random_access { };
struct sink_tag : device_tag, output { };

struct seekable
    : virtual input_seekable, 
      virtual output_seekable, one_head
{ };

struct seekable_device_tag : virtual device_tag, seekable { };

template<typename T> struct unwrap_reference
{
    typedef T type;
};

template<typename T>
struct unwrapped_type 
    : unwrap_reference<T>
    { };

template<typename T>
struct member_category { typedef typename T::category type; };


template<
      bool C
    , typename T1
    , typename T2
    >
struct if_c
{
    typedef T1 type;
};

template<
      typename MARS_MPL_AUX_NA_PARAM(T1)
    , typename MARS_MPL_AUX_NA_PARAM(T2)
    , typename MARS_MPL_AUX_NA_PARAM(T3)
    >
struct if_
{
 private:
    // agurt, 02/jan/03: two-step 'type' definition for the sake of aCC 
    typedef if_c<static_cast<bool>(T1::value)
        , T2
        , T3
        > almost_type_;
 
 public:
    typedef typename almost_type_::type type;
    
    // BOOST_MPL_AUX_LAMBDA_SUPPORT(3,if_,(T1,T2,T3))
};

template<
      typename C = na
    , typename F1 = na
    , typename F2 = na
    >
struct eval_if: if_<C,F1,F2>::type {
};

// template<
//       typename MARS_MPL_AUX_NA_PARAM(T1)
//     , typename MARS_MPL_AUX_NA_PARAM(T2)
//     , typename T3 = true_, typename T4 = true_, typename T5 = true_
//     >
// struct and_

//     : and_impl<
//           BOOST_MPL_AUX_NESTED_TYPE_WKND(T1)::value
//         , T2, T3, T4, T5
//         >

// {
// };

// template<typename T>
// struct is_std_io
//     : std::logical_or< is_istream<T>, is_ostream<T>, is_streambuf<T> >
//     { };


// template<typename T>
// struct category_of {
//     template<typename U>
//     struct member_category { 
//         typedef typename U::category type; 
//     };
//     typedef typename unwrapped_type<T>::type U;

//     typedef typename eval_if<
//                 std::logical_and<
//                     is_std_io<U>,
//                     mpl::not_< detail::is_boost<U> >
//                 >,
//                 iostreams::select<  // Disambiguation for Tru64
//                     is_filebuf<U>,        filebuf_tag,
//                     is_ifstream<U>,       ifstream_tag,
//                     is_ofstream<U>,       ofstream_tag,
//                     is_fstream<U>,        fstream_tag,
//                     is_stringbuf<U>,      stringbuf_tag,
//                     is_istringstream<U>,  istringstream_tag,
//                     is_ostringstream<U>,  ostringstream_tag,
//                     is_stringstream<U>,   stringstream_tag,
//                     is_streambuf<U>,      generic_streambuf_tag,
//                     is_iostream<U>,       generic_iostream_tag,
//                     is_istream<U>,        generic_istream_tag, 
//                     is_ostream<U>,        generic_ostream_tag
//                 >,
//                 detail::member_category<U>
//             >::type type;
// };

// template<typename T>
// struct close_tag {
//     typedef typename category_of<T>::type             category;
//     typedef typename unwrapped_type<T>::type  unwrapped;
//     typedef typename
//             iostreams::select<
//                 mpl::not_< is_convertible<category, closable_tag> >,
//                 any_tag,
//                 mpl::or_<
//                     is_boost_stream<unwrapped>,
//                     is_boost_stream_buffer<unwrapped>
//                 >,
//                 close_boost_stream,
//                 mpl::or_<
//                     is_filtering_stream<unwrapped>,
//                     is_filtering_streambuf<unwrapped>
//                 >,
//                 close_filtering_stream,
//                 mpl::or_<
//                     is_convertible<category, two_sequence>,
//                     is_convertible<category, dual_use>
//                 >,
//                 two_sequence,
//                 else_,
//                 closable_tag
//             >::type type;
// };


template<long C_> // 'long' is intentional here
struct not_impl
    : bool_<!C_> {};


template<typename MARS_MPL_AUX_NA_PARAM(T)>
struct not_
    : not_impl<T::type::value>
{
    // BOOST_MPL_AUX_LAMBDA_SUPPORT(1,not_,(T))
};


struct custom_tag { };


template<typename T>
struct operations : custom_tag { };

template<typename T>
struct is_custom
    : not_<std::is_base_of< custom_tag, operations<T> >>
    { };

class mapped_file_base {
public:
    enum mapmode {
        readonly = 1,
        readwrite = 2,
        priv = 4
    };
};


struct mapped_file_params_base {
    mapped_file_params_base()
        : flags(static_cast<mapped_file_base::mapmode>(0)), 
          mode(), offset(0), length(static_cast<std::size_t>(-1)), 
          new_file_size(0), hint(0)
        { }
private:
    friend class mapped_file_impl;
    void normalize();
public:
    mapped_file_base::mapmode   flags;
    std::ios::openmode          mode;  // Deprecated
    stream_offset               offset;
    std::size_t                 length;
    stream_offset               new_file_size;
    const char*                 hint;
};



template<typename Path>
struct basic_mapped_file_params 
    : mapped_file_params_base 
{
    typedef mapped_file_params_base base_type;

    // Default constructor
    basic_mapped_file_params() { }

    // Construction from a Path
    explicit basic_mapped_file_params(const Path& p) : path(p) { }

    // Construction from a path of a different type
    template<typename PathT>
    explicit basic_mapped_file_params(const PathT& p) : path(p) { }

    // Copy constructor
    basic_mapped_file_params(const basic_mapped_file_params& other)
        : base_type(other), path(other.path)
        { }

    // Templated copy constructor
    template<typename PathT>
    basic_mapped_file_params(const basic_mapped_file_params<PathT>& other)
        : base_type(other), path(other.path)
        { }

    typedef Path  path_type;
    Path          path;
};

class mapped_file_impl {
public:
    typedef std::size_t                     size_type;
    typedef basic_mapped_file_params<mars::filesystem::path>  param_type;
    typedef mapped_file_base::mapmode       mapmode;
    static const size_type max_length = static_cast<size_type>(-1);;
    mapped_file_impl();
    ~mapped_file_impl();
    void open(param_type p);
    bool is_open() const { return (data_ != 0 && handle_ >= 0); }
    void close();
    bool error() const { return error_; }
    mapmode flags() const { return params_.flags; }
    std::size_t size() const { return size_; }
    char* data() const { return data_; }
    void resize(stream_offset new_size);
    static int alignment();
private:
    void open_file(param_type p);
    void try_map_file(param_type p);
    void map_file(param_type& p);
    bool unmap_file();
    void clear(bool error);
    void cleanup_and_throw(const char* msg);
    param_type     params_;
    char*          data_;
    stream_offset  size_;
    file_handle    handle_;
    bool           error_;
};

// template<typename T>
// struct close_impl
//     : if_<
//           is_custom<T>, 
//           operations<T>, 
//           close_impl<typename close_tag<T>::type>>::type
//     { };

// template<>
// struct close_impl<closable_tag> {
//     template<typename T>
//     static void close(T& t, std::ios::openmode which)
//     {
//         // typedef typename category_of<T>::type category;
//         // const bool in =  is_convertible<category, input>::value &&
//         //                 !is_convertible<category, output>::value;
//         if (which == std::ios::in) {
//             t.close();
//         }
//     }
//     // template<typename T, typename Sink>
//     // static void close(T& t, Sink& snk, openmode which)
//     // {
//     //     // typedef typename category_of<T>::type category;
//     //     // const bool in =  is_convertible<category, input>::value &&
//     //     //                 !is_convertible<category, output>::value;
//     //     if (which == in) {
//     //         non_blocking_adapter<Sink> nb(snk);
//     //         t.close(nb);
//     //     }
//     // }
// }; 

class mapped_file_source;

// template<>
// struct operations<mapped_file_source>
//     : close_impl<closable_tag>
// {
//     static std::pair<char*, char*>
//     input_sequence(mapped_file_source& src)
//     {
//         return std::make_pair( const_cast<char*>(src.begin()),
//                                const_cast<char*>(src.end()) );
//     }
// };

class mapped_file_source : public mapped_file_base {
private:
    struct safe_bool_helper { int x; };
    typedef int safe_bool_helper::*                 safe_bool;
    typedef mapped_file_impl                        impl_type;
    typedef basic_mapped_file_params<mars::filesystem::path>  param_type;
    friend class mapped_file;
    friend class mapped_file_impl;
    // friend struct operations<mapped_file_source>;
public:
    typedef char                                    char_type;
    struct category
        : public source_tag,
          public direct_tag,
          public closable_tag
        { };
    typedef std::size_t                             size_type;
    typedef const char*                             iterator;
    static const size_type max_length = static_cast<size_type>(-1);

    // Default constructor
    mapped_file_source();

    // Constructor taking a parameters object
    template<typename Path>
    explicit mapped_file_source(const basic_mapped_file_params<Path>& p);

    // Constructor taking a list of parameters
    template<typename Path>
    explicit mapped_file_source( const Path& path, size_type length = max_length, intmax_t offset = 0 );

    mapped_file_source(const mapped_file_source& other);

    //--------------Stream interface------------------------------------------//

    template<typename Path>
    void open(const basic_mapped_file_params<Path>& p);

    template<typename Path>
    void open( const Path& path, size_type length = max_length, intmax_t offset = 0 );

    bool is_open() const;
    void close();
    operator safe_bool() const;
    bool operator!() const;
    mapmode flags() const;

    //--------------Container interface---------------------------------------//

    size_type size() const;
    const char* data() const;
    iterator begin() const;
    iterator end() const;

    //--------------Query admissible offsets----------------------------------//

    // Returns the allocation granularity for virtual memory. Values passed
    // as offsets must be multiples of this value.
    static int alignment();

private:
    void init();
    void open_impl(const param_type& p);

    std::shared_ptr<impl_type> pimpl_;
};

class mapped_file : public mapped_file_base {
private:
    typedef mapped_file_source                      delegate_type;
    typedef delegate_type::safe_bool                safe_bool;
    typedef basic_mapped_file_params<path>  param_type;
    friend class mapped_file_sink;
public:
    typedef char                                    char_type;
    struct category
        : public seekable_device_tag,
          public direct_tag,
          public closable_tag
        { };
    typedef mapped_file_source::size_type           size_type;
    typedef char*                                   iterator;
    typedef const char*                             const_iterator;
    MARS_STATIC_CONSTANT(size_type, max_length = delegate_type::max_length);

    // Default constructor
    mapped_file() { }

    // Construstor taking a parameters object
    template<typename Path>
    explicit mapped_file(const basic_mapped_file_params<Path>& p);

    // Constructor taking a list of parameters
    template<typename Path>
    mapped_file( const Path& path,
                 mapmode flags,
                 size_type length = max_length,
                 stream_offset offset = 0 );

    // Constructor taking a list of parameters, including a 
    // std::ios_base::openmode (deprecated)
    template<typename Path>
    explicit mapped_file( const Path& path,
                          std::ios::openmode mode =
                               std::ios::in |  std::ios::out,
                          size_type length = max_length,
                          stream_offset offset = 0 );

    // Copy Constructor
    mapped_file(const mapped_file& other);

    //--------------Conversion to mapped_file_source (deprecated)-------------//

    operator mapped_file_source&() { return delegate_; }
    operator const mapped_file_source&() const { return delegate_; }

    //--------------Stream interface------------------------------------------//

    // open overload taking a parameters object
    template<typename Path>
    void open(const basic_mapped_file_params<Path>& p);

    // open overload taking a list of parameters
    template<typename Path>
    void open( const Path& path,
               mapmode mode,
               size_type length = max_length,
               stream_offset offset = 0 );

    // open overload taking a list of parameters, including a 
    // std::ios_base::openmode (deprecated)
    template<typename Path>
    void open( const Path& path,
                std::ios::openmode mode =
                    std::ios::in | std::ios::out,
               size_type length = max_length,
               stream_offset offset = 0 );

    bool is_open() const { return delegate_.is_open(); }
    void close() { delegate_.close(); }
    operator safe_bool() const { return delegate_; }
    bool operator!() const { return !delegate_; }
    mapmode flags() const { return delegate_.flags(); }

    //--------------Container interface---------------------------------------//

    size_type size() const { return delegate_.size(); }
    char* data() const;
    const char* const_data() const { return delegate_.data(); }
    iterator begin() const { return data(); }
    const_iterator const_begin() const { return const_data(); }
    iterator end() const { return data() + size(); }
    const_iterator const_end() const { return const_data() + size(); }

    //--------------Query admissible offsets----------------------------------//

    // Returns the allocation granularity for virtual memory. Values passed
    // as offsets must be multiples of this value.
    static int alignment() { return mapped_file_source::alignment(); }

    //--------------File access----------------------------------------------//

    void resize(stream_offset new_size);
private:
    delegate_type delegate_;
};

//------------------Definition of mapped_file_sink----------------------------//

class mapped_file_sink : private mapped_file {
public:
    using mapped_file::mapmode;
    using mapped_file::readonly;
    using mapped_file::readwrite;
    using mapped_file::priv;
    using mapped_file::char_type;
    struct category
        : public sink_tag,
          public direct_tag,
          public closable_tag
        { };
    using mapped_file::size_type;
    using mapped_file::iterator;
    using mapped_file::max_length;
    using mapped_file::is_open;
    using mapped_file::close;
    using mapped_file::operator safe_bool;
    using mapped_file::operator !;
    using mapped_file::flags;
    using mapped_file::size;
    using mapped_file::data;
    using mapped_file::begin;
    using mapped_file::end;
    using mapped_file::alignment;
    using mapped_file::resize;

    // Default constructor
    mapped_file_sink() { }

    // Constructor taking a parameters object
    template<typename Path>
    explicit mapped_file_sink(const basic_mapped_file_params<Path>& p);

    // Constructor taking a list of parameters
    template<typename Path>
    explicit mapped_file_sink( const Path& path,
                               size_type length = max_length,
                                intmax_t offset = 0,
                               mapmode flags = readwrite );

    // Copy Constructor
    mapped_file_sink(const mapped_file_sink& other);

    // open overload taking a parameters object
    template<typename Path>
    void open(const basic_mapped_file_params<Path>& p);

    // open overload taking a list of parameters
    template<typename Path>
    void open( const Path& path,
               size_type length = max_length,
               intmax_t offset = 0,
               mapmode flags = readwrite );
};

}; // namespace mars
}; // namespace filesystem



#endif