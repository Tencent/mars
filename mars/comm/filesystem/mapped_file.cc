#include "mapped_file.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>

#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace filesystem {

    
mapped_file_impl::mapped_file_impl() { clear(false); }

mapped_file_impl::~mapped_file_impl()  {
    close(); 
};

void mapped_file_impl::open(param_type p)
{
    if (is_open()) {
        xerror2(TSF"file already open");
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
    if(handle_ >= 0) {
        error = ::close(handle_) != 0 || error;
    }
    clear(error);
    if (error) {
        xerror2(TSF"failed closing mapped file: %_", error);
    }
}

void mapped_file_impl::resize(stream_offset new_size)
{
    if (!is_open()) {
        xerror2("file is closed");
        return;
    }
    if (flags() & mapped_file_base::priv) {
        xerror2("can't resize private mapped file");
        return;
    }
    if (!(flags() & mapped_file_base::readwrite)) {
        xerror2("can't resize readonly mapped file");
        return;
    }
    if (params_.offset >= new_size) {
        xerror2("can't resize below mapped offset");
        return;
    }
    if (!unmap_file()) {
        cleanup_and_throw("failed unmapping file");
        return;
    }
    if (ftruncate(handle_, new_size) == -1) {
        cleanup_and_throw("failed resizing mapped file");
        return;
    }
    size_ = new_size;
    param_type p(params_);
    map_file(p);  // May modify p.hint
    params_ = p;
}

int mapped_file_impl::alignment() {
    return static_cast<int>(sysconf(_SC_PAGESIZE));
}

void mapped_file_impl::open_file(param_type p)
{
    bool readonly = p.flags != mapped_file_base::readwrite;
    // Open file
    int flags = (readonly ? O_RDONLY : O_RDWR);
    if (p.new_file_size != 0 && !readonly)
        flags |= (O_CREAT | O_TRUNC);
    errno = 0;
    handle_ = ::open(p.path.c_str(), flags, S_IRWXU);
    if (errno != 0) {
        cleanup_and_throw("failed opening file");
        return;
    }

    //--------------Set file size---------------------------------------------//

    if (p.new_file_size != 0 && !readonly)
        if (ftruncate(handle_, p.new_file_size) == -1) {
            cleanup_and_throw("failed setting file size");
            return;
        }

    //--------------Determine file size---------------------------------------//

    bool success = true;
    if (p.length != max_length) {
        size_ = p.length;
    } else {
        struct stat info;
        success = ::fstat(handle_, &info) != -1;
        size_ = info.st_size;
    }
    if (!success) {
        cleanup_and_throw("failed querying file size");
        return;
    }
}

void mapped_file_impl::try_map_file(param_type p)
{
    bool priv = p.flags == mapped_file_base::priv;
    bool readonly = p.flags == mapped_file_base::readonly;

    void* data = 
        ::mmap( 
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
    data_ = static_cast<char*>(data);
}

void mapped_file_impl::map_file(param_type& p)
{
    try_map_file(p);
    // BOOST_CATCH (const std::exception&) {
    //     if (p.hint) {
    //         p.hint = 0;
    //         try_map_file(p);
    //     } else {
    //         BOOST_RETHROW;
    //     }
    // }
}

bool mapped_file_impl::unmap_file()
{
    return ::munmap(data_, size_) == 0;
}

void mapped_file_impl::clear(bool error)
{
    params_ = param_type();
    data_ = 0;
    size_ = 0;
    handle_ = -1;
    error_ = error;
}

// Called when an error is encountered during the execution of open_file or
// map_file
void mapped_file_impl::cleanup_and_throw(const char* msg)
{
    int error = errno;
    if (handle_ >= 0)
        ::close(handle_);
    errno = error;
    clear(true);
    xerror2(TSF"clear error: %_", msg);
}

//------------------Implementation of mapped_file_params_base-----------------//

void mapped_file_params_base::normalize()
{
    if (mode && flags) {
        xerror2("at most one of 'mode' and 'flags' may be specified");
    }
    if (flags) {
        switch (flags) {
        case mapped_file_base::readonly:
        case mapped_file_base::readwrite:
        case mapped_file_base::priv:
            break;
        default:
            xerror2("invalid flags");
        }
    } else {
        flags = (mode & std::ios::out) ? 
            mapped_file_base::readwrite :
            mapped_file_base::readonly;
        mode = std::ios::openmode();
    }
    if (offset < 0) {
        xerror2("invalid offset");
    }
    if (new_file_size < 0) {
        xerror2("nvalid new file size");
    }
}



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
{ return mapped_file_impl::alignment(); }

void mapped_file_source::init() { pimpl_.reset(new impl_type); }

void mapped_file_source::open_impl(const param_type& p)
{ pimpl_->open(p); }


template<typename Path>
mapped_file_source::mapped_file_source(const basic_mapped_file_params<Path>& p)
{ init(); open(p); }

template<typename Path>
mapped_file_source::mapped_file_source(const Path& path, size_type length, intmax_t offset)
{ init(); open(path, length, offset); }

template<typename Path>
void mapped_file_source::open(const basic_mapped_file_params<Path>& p)
{
    param_type params(p);
    if (params.flags) {
        if (params.flags != mapped_file::readonly) {
            xerror2(TSF"open invalid flags: %_", params.flags);
        }
    } else {
        if (params.mode & std::ios::out) {
            xerror2(TSF"open invalid mode: %_", params.mode);
        }
        params.mode |= std::ios::in;
    }
    open_impl(params);
}

template<typename Path>
void mapped_file_source::open(const Path& path, size_type length, intmax_t offset)
{
    param_type p(path);
    p.length = length;
    p.offset = offset;
    open(p);
}

mapped_file::mapped_file(const mapped_file& other)
    : delegate_(other.delegate_)
    { }

void mapped_file::resize(stream_offset new_size)
{ delegate_.pimpl_->resize(new_size); }

char* mapped_file::data() const {
    return (flags() != readonly) ? const_cast<char*>(delegate_.data()) : 0; 
}

template<typename Path>
mapped_file::mapped_file(const basic_mapped_file_params<Path>& p)
{ open(p); }

template<typename Path>
mapped_file::mapped_file( 
    const Path& path, mapmode flags, 
    size_type length, stream_offset offset )
{ open(path, flags, length, offset); }

template<typename Path>
mapped_file::mapped_file( const Path& path, std::ios::openmode mode, 
    size_type length, stream_offset offset )
{ open(path, mode, length, offset); }

template<typename Path>
void mapped_file::open(const basic_mapped_file_params<Path>& p)
{ delegate_.open_impl(p); }

template<typename Path>
void mapped_file::open( 
    const Path& path, mapmode flags, 
    size_type length, stream_offset offset )
{
    param_type p(path);
    p.flags = flags;
    p.length = length;
    p.offset = offset;
    open(p);
}

void mapped_file::open(const basic_mapped_file_params<mars::filesystem::path>& p) {
    delegate_.open_impl(p);
}

template<typename Path>
void mapped_file::open( const Path& path, std::ios::openmode mode, 
    size_type length, stream_offset offset )
{
    param_type p(path);
    p.mode = mode;
    p.length = length;
    p.offset = offset;
    open(p);
}

mapped_file_sink::mapped_file_sink(const mapped_file_sink& other)
    : mapped_file(static_cast<const mapped_file&>(other)) { }


}; // namespace filesystem
}; // namespace mars