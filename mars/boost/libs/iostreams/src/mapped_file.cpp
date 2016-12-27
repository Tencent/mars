// (C) Copyright Craig Henderson 2002 'boost/memmap.hpp' from sandbox
// (C) Copyright Jonathan Turkanis 2004.
// (C) Copyright Jonathan Graehl 2004.
// (C) Copyright Jorge Lodos 2008.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// Define BOOST_IOSTREAMS_SOURCE so that <boost/iostreams/detail/config.hpp>
// knows that we are building the library (possibly exporting code), rather
// than using it (possibly importing code).
#define BOOST_IOSTREAMS_SOURCE

#include <cassert>
#include <boost/iostreams/detail/config/rtl.hpp>
#include <boost/iostreams/detail/config/windows_posix.hpp>
#include <boost/iostreams/detail/file_handle.hpp>
#include <boost/iostreams/detail/system_failure.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/throw_exception.hpp>
#include <boost/detail/no_exceptions_support.hpp>


#ifdef BOOST_IOSTREAMS_WINDOWS
# define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
# include <windows.h>

#if UWP

DWORD SetFilePointer(
	_In_ HANDLE hFile,
	_In_ LONG lDistanceToMove,
	_Inout_opt_ PLONG lpDistanceToMoveHigh,
	_In_ DWORD dwMoveMethod
	);


HANDLE
CreateFileW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,//
	_In_opt_ HANDLE hTemplateFile
	);




DWORD GetFileSize(
	_In_      HANDLE  hFile,
	_Out_opt_ LPDWORD lpFileSizeHigh
	);


HANDLE
WINAPI
CreateFileA(
	_In_ LPCSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
	);


HANDLE
CreateFileMappingA(
	_In_     HANDLE hFile,
	_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
	_In_     DWORD flProtect,
	_In_     DWORD dwMaximumSizeHigh,
	_In_     DWORD dwMaximumSizeLow,
	_In_opt_ LPCSTR lpName
	);

LPVOID
WINAPI
MapViewOfFileEx(
	_In_ HANDLE hFileMappingObject,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwFileOffsetHigh,
	_In_ DWORD dwFileOffsetLow,
	_In_ SIZE_T dwNumberOfBytesToMap,
	_In_opt_ LPVOID lpBaseAddress
	);


#endif 


#else
# include <errno.h>
# include <fcntl.h>
# include <sys/mman.h>      // mmap, munmap.
# include <sys/stat.h>
# include <sys/types.h>     // struct stat.
# include <unistd.h>        // sysconf.
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost { namespace iostreams {

namespace detail {

// Class containing the platform-sepecific implementation
// Invariant: The members params_, data_, size_, handle_ (and mapped_handle_ 
// on Windows) either
//    - all have default values (or INVALID_HANDLE_VALUE for
//      Windows handles), or
//    - all have values reflecting a successful mapping.
// In the first case, error_ may be true, reflecting a recent unsuccessful
// open or close attempt; in the second case, error_ is always false.
class mapped_file_impl {
public:
    typedef mapped_file_source::size_type   size_type;
    typedef mapped_file_source::param_type  param_type;
    typedef mapped_file_source::mapmode     mapmode;
    BOOST_STATIC_CONSTANT(
        size_type, max_length =  mapped_file_source::max_length);
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
#ifdef BOOST_IOSTREAMS_WINDOWS
    file_handle    mapped_handle_;
#endif
    bool           error_;
};

mapped_file_impl::mapped_file_impl() { clear(false); }

mapped_file_impl::~mapped_file_impl() 
{ BOOST_TRY { close(); } BOOST_CATCH (...) { } BOOST_CATCH_END }

void mapped_file_impl::open(param_type p)
{
    if (is_open()) {
        mars_boost::throw_exception(BOOST_IOSTREAMS_FAILURE("file already open"));
        return;
    }
    p.normalize();
    open_file(p);
    map_file(p);  // May modify p.hint
    params_ = p;
}

void mapped_file_impl::close()
{
    if (data_ == 0)
        return;
    bool error = false;
    error = !unmap_file() || error;
    #ifdef BOOST_IOSTREAMS_WINDOWS
    if(handle_ != INVALID_HANDLE_VALUE)
    #else
    if(handle_ >= 0)
    #endif
    {
        error =
            #ifdef BOOST_IOSTREAMS_WINDOWS
                !::CloseHandle(handle_)
            #else
                ::close(handle_) != 0
            #endif
                || error;
    }
    clear(error);
    if (error)
        throw_system_failure("failed closing mapped file");
}

void mapped_file_impl::resize(stream_offset new_size)
{
    if (!is_open()) {
        mars_boost::throw_exception(BOOST_IOSTREAMS_FAILURE("file is closed"));
        return;
    }
    if (flags() & mapped_file::priv) {
        mars_boost::throw_exception(
            BOOST_IOSTREAMS_FAILURE("can't resize private mapped file")
        );
        return;
    }
    if (!(flags() & mapped_file::readwrite)) {
        mars_boost::throw_exception(
            BOOST_IOSTREAMS_FAILURE("can't resize readonly mapped file")
        );
        return;
    }
    if (params_.offset >= new_size) {
        mars_boost::throw_exception(
            BOOST_IOSTREAMS_FAILURE("can't resize below mapped offset")
        );
        return;
    }
    if (!unmap_file()) {
        cleanup_and_throw("failed unmapping file");
        return;
    }
#ifdef BOOST_IOSTREAMS_WINDOWS
    stream_offset offset = ::SetFilePointer(handle_, 0, NULL, FILE_CURRENT);
    if (offset == INVALID_SET_FILE_POINTER && ::GetLastError() != NO_ERROR) {
         cleanup_and_throw("failed querying file pointer");
        return;
    }
    LONG sizehigh = (new_size >> (sizeof(LONG) * 8));
    LONG sizelow = (new_size & 0xffffffff);
    DWORD result = ::SetFilePointer(handle_, sizelow, &sizehigh, FILE_BEGIN);
    if ((result == INVALID_SET_FILE_POINTER && ::GetLastError() != NO_ERROR)
        || !::SetEndOfFile(handle_)) {
        cleanup_and_throw("failed resizing mapped file");
        return;
    }
    sizehigh = (offset >> (sizeof(LONG) * 8));
    sizelow = (offset & 0xffffffff);
    ::SetFilePointer(handle_, sizelow, &sizehigh, FILE_BEGIN);
#else
    if (BOOST_IOSTREAMS_FD_TRUNCATE(handle_, new_size) == -1) {
        cleanup_and_throw("failed resizing mapped file");
        return;
    }
#endif
    size_ = new_size;
    param_type p(params_);
    map_file(p);  // May modify p.hint
    params_ = p;
}

int mapped_file_impl::alignment()
{
#ifdef BOOST_IOSTREAMS_WINDOWS
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);
    return static_cast<int>(info.dwAllocationGranularity);
#else
    return static_cast<int>(sysconf(_SC_PAGESIZE));
#endif
}

void mapped_file_impl::open_file(param_type p)
{
    bool readonly = p.flags != mapped_file::readwrite;
#ifdef BOOST_IOSTREAMS_WINDOWS

    // Open file
    DWORD dwDesiredAccess =
        readonly ?
            GENERIC_READ :
            (GENERIC_READ | GENERIC_WRITE);
    DWORD dwCreationDisposition = (p.new_file_size != 0 && !readonly) ? 
        CREATE_ALWAYS : 
        OPEN_EXISTING;
    DWORD dwFlagsandAttributes =
        readonly ?
            FILE_ATTRIBUTE_READONLY :
            FILE_ATTRIBUTE_TEMPORARY;
    handle_ = p.path.is_wide() ?
        ::CreateFileW( 
            p.path.c_wstr(),
            dwDesiredAccess,
            FILE_SHARE_READ,
            NULL,
            dwCreationDisposition,
            dwFlagsandAttributes,
            NULL ) :
        ::CreateFileA( 
            p.path.c_str(),
            dwDesiredAccess,
            FILE_SHARE_READ,
            NULL,
            dwCreationDisposition,
            dwFlagsandAttributes,
            NULL );
    if (handle_ == INVALID_HANDLE_VALUE) {
        cleanup_and_throw("failed opening file");
        return;
    }

    // Set file size
    if (p.new_file_size != 0 && !readonly) {
        LONG sizehigh = (p.new_file_size >> (sizeof(LONG) * 8));
        LONG sizelow = (p.new_file_size & 0xffffffff);
        DWORD result = ::SetFilePointer(handle_, sizelow, &sizehigh, FILE_BEGIN);
        if ((result == INVALID_SET_FILE_POINTER && ::GetLastError() != NO_ERROR)
            || !::SetEndOfFile(handle_)) {
            cleanup_and_throw("failed setting file size");
            return;
        }
    }

    // Determine file size. Dynamically locate GetFileSizeEx for compatibility
    // with old Platform SDK (thanks to Pavel Vozenilik).
    typedef BOOL (WINAPI *func)(HANDLE, PLARGE_INTEGER);
#ifdef UWP
	func get_size = &GetFileSizeEx;
#else
    HMODULE hmod = ::GetModuleHandleA("kernel32.dll");
    func get_size =
        reinterpret_cast<func>(::GetProcAddress(hmod, "GetFileSizeEx"));
#endif
    if (get_size) {
        LARGE_INTEGER info;
        if (get_size(handle_, &info)) {
            mars_boost::intmax_t size =
                ( (static_cast<mars_boost::intmax_t>(info.HighPart) << 32) |
                  info.LowPart );
            size_ =
                static_cast<std::size_t>(
                    p.length != max_length ?
                        std::min<mars_boost::intmax_t>(p.length, size) :
                        size
                );
        } else {
            cleanup_and_throw("failed querying file size");
            return;
        }
    } else {
        DWORD hi;
        DWORD low;
        if ( (low = ::GetFileSize(handle_, &hi))
                 !=
             INVALID_FILE_SIZE )
        {
            mars_boost::intmax_t size =
                (static_cast<mars_boost::intmax_t>(hi) << 32) | low;
            size_ =
                static_cast<std::size_t>(
                    p.length != max_length ?
                        std::min<mars_boost::intmax_t>(p.length, size) :
                        size
                );
        } else {
            cleanup_and_throw("failed querying file size");
            return;
        }
    }
#else // #ifdef BOOST_IOSTREAMS_WINDOWS

    // Open file
    int flags = (readonly ? O_RDONLY : O_RDWR);
    if (p.new_file_size != 0 && !readonly)
        flags |= (O_CREAT | O_TRUNC);
    #ifdef _LARGEFILE64_SOURCE
        flags |= O_LARGEFILE;
    #endif
    errno = 0;
    handle_ = ::open(p.path.c_str(), flags, S_IRWXU);
    if (errno != 0) {
        cleanup_and_throw("failed opening file");
        return;
    }

    //--------------Set file size---------------------------------------------//

    if (p.new_file_size != 0 && !readonly)
        if (BOOST_IOSTREAMS_FD_TRUNCATE(handle_, p.new_file_size) == -1) {
            cleanup_and_throw("failed setting file size");
            return;
        }

    //--------------Determine file size---------------------------------------//

    bool success = true;
    if (p.length != max_length) {
        size_ = p.length;
    } else {
        struct BOOST_IOSTREAMS_FD_STAT info;
        success = ::BOOST_IOSTREAMS_FD_FSTAT(handle_, &info) != -1;
        size_ = info.st_size;
    }
    if (!success) {
        cleanup_and_throw("failed querying file size");
        return;
    }
#endif // #ifdef BOOST_IOSTREAMS_WINDOWS
}

void mapped_file_impl::try_map_file(param_type p)
{
    bool priv = p.flags == mapped_file::priv;
    bool readonly = p.flags == mapped_file::readonly;
#ifdef BOOST_IOSTREAMS_WINDOWS

    // Create mapping
    DWORD protect = priv ? 
        PAGE_WRITECOPY : 
        readonly ? 
            PAGE_READONLY : 
            PAGE_READWRITE;
    mapped_handle_ = 
        ::CreateFileMappingA( 
            handle_, 
            NULL,
            protect,
            0, 
            0, 
            NULL );
    if (mapped_handle_ == NULL)
        cleanup_and_throw("failed create mapping");

    // Access data
    DWORD access = priv ? 
        FILE_MAP_COPY : 
        readonly ? 
            FILE_MAP_READ : 
            FILE_MAP_WRITE;
    void* data =
        ::MapViewOfFileEx( 
            mapped_handle_,
            access,
            (DWORD) (p.offset >> 32),
            (DWORD) (p.offset & 0xffffffff),
            size_ != max_length ? size_ : 0, 
            (LPVOID) p.hint );
    if (!data) {
        cleanup_and_throw("failed mapping view");
        return;
    }
#else
    void* data = 
        ::BOOST_IOSTREAMS_FD_MMAP( 
            const_cast<char*>(p.hint), 
            size_,
            readonly ? PROT_READ : (PROT_READ | PROT_WRITE),
            priv ? MAP_PRIVATE : MAP_SHARED,
            handle_, 
            p.offset );
    if (data == MAP_FAILED) {
        cleanup_and_throw("failed mapping file");
        return;
    }
#endif
    data_ = static_cast<char*>(data);
}

void mapped_file_impl::map_file(param_type& p)
{
    BOOST_TRY {
        try_map_file(p);
    } BOOST_CATCH (const std::exception&) {
        if (p.hint) {
            p.hint = 0;
            try_map_file(p);
        } else {
            BOOST_RETHROW;
        }
    }
    BOOST_CATCH_END
}

bool mapped_file_impl::unmap_file()
{
#ifdef BOOST_IOSTREAMS_WINDOWS
    bool error = false;
    error = !::UnmapViewOfFile(data_) || error;
    error = !::CloseHandle(mapped_handle_) || error;
    mapped_handle_ = NULL;
    return !error;
#else
    return ::munmap(data_, size_) == 0;
#endif
}

void mapped_file_impl::clear(bool error)
{
    params_ = param_type();
    data_ = 0;
    size_ = 0;
#ifdef BOOST_IOSTREAMS_WINDOWS
    handle_ = INVALID_HANDLE_VALUE;
    mapped_handle_ = NULL;
#else
    handle_ = -1;
#endif
    error_ = error;
}

// Called when an error is encountered during the execution of open_file or
// map_file
void mapped_file_impl::cleanup_and_throw(const char* msg)
{
#ifdef BOOST_IOSTREAMS_WINDOWS
    DWORD error = GetLastError();
    if (mapped_handle_ != NULL)
        ::CloseHandle(mapped_handle_);
    if (handle_ != INVALID_HANDLE_VALUE)
        ::CloseHandle(handle_);
    SetLastError(error);
#else
    int error = errno;
    if (handle_ >= 0)
        ::close(handle_);
    errno = error;
#endif
    clear(true);
    BOOST_TRY {
    	mars_boost::iostreams::detail::throw_system_failure(msg);
    } BOOST_CATCH(...) {
    } BOOST_CATCH_END
}

//------------------Implementation of mapped_file_params_base-----------------//

void mapped_file_params_base::normalize()
{
    if (mode && flags)
        mars_boost::throw_exception(BOOST_IOSTREAMS_FAILURE(
            "at most one of 'mode' and 'flags' may be specified"
        ));
    if (flags) {
        switch (flags) {
        case mapped_file::readonly:
        case mapped_file::readwrite:
        case mapped_file::priv:
            break;
        default:
            mars_boost::throw_exception(BOOST_IOSTREAMS_FAILURE("invalid flags"));
        }
    } else {
        flags = (mode & BOOST_IOS::out) ? 
            mapped_file::readwrite :
            mapped_file::readonly;
        mode = BOOST_IOS::openmode();
    }
    if (offset < 0)
        mars_boost::throw_exception(BOOST_IOSTREAMS_FAILURE("invalid offset"));
    if (new_file_size < 0)
        mars_boost::throw_exception(
            BOOST_IOSTREAMS_FAILURE("invalid new file size")
        );
}

} // End namespace detail.

//------------------Implementation of mapped_file_source----------------------//

mapped_file_source::mapped_file_source() 
    : pimpl_(new impl_type)
    { }

mapped_file_source::mapped_file_source(const mapped_file_source& other)
    : pimpl_(other.pimpl_)
    { }

bool mapped_file_source::is_open() const
{ return pimpl_->is_open(); }

void mapped_file_source::close() { pimpl_->close(); }

// safe_bool is explicitly qualified below to please msvc 7.1
mapped_file_source::operator mapped_file_source::safe_bool() const
{ return pimpl_->error() ? &safe_bool_helper::x : 0; }

bool mapped_file_source::operator!() const
{ return pimpl_->error(); }

mapped_file_source::mapmode mapped_file_source::flags() const 
{ return pimpl_->flags(); }

mapped_file_source::size_type mapped_file_source::size() const
{ return pimpl_->size(); }

const char* mapped_file_source::data() const { return pimpl_->data(); }

const char* mapped_file_source::begin() const { return data(); }

const char* mapped_file_source::end() const { return data() + size(); }
int mapped_file_source::alignment()
{ return detail::mapped_file_impl::alignment(); }

void mapped_file_source::init() { pimpl_.reset(new impl_type); }

void mapped_file_source::open_impl(const param_type& p)
{ pimpl_->open(p); }

//------------------Implementation of mapped_file-----------------------------//

mapped_file::mapped_file(const mapped_file& other)
    : delegate_(other.delegate_)
    { }

void mapped_file::resize(stream_offset new_size)
{ delegate_.pimpl_->resize(new_size); }

//------------------Implementation of mapped_file_sink------------------------//

mapped_file_sink::mapped_file_sink(const mapped_file_sink& other)
    : mapped_file(static_cast<const mapped_file&>(other))
    { }

//----------------------------------------------------------------------------//

} } // End namespaces iostreams, boost.
