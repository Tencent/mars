#pragma once

#include <utility>

#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/xlogger/xlogger.h"

#ifndef NDEBUG
#include "mars/comm/fd_info.h"
#endif

namespace internal {
struct SocketTraits {
    static SOCKET InvalidValue() {
        return INVALID_SOCKET;
    }
    static void Free(SOCKET sock) {
#ifndef NDEBUG
        xassert2(mars::comm::FDInfo::QueryFD(sock).IsSocket());
#endif
        socket_close(sock);
    }
};

struct FDTraits {
    static int InvalidValue() {
        return -1;
    }
    static void Free(int fd) {
#ifndef NDEBUG
        xassert2(mars::comm::FDInfo::QueryFD(fd).IsFile());
#endif
        close(fd);
    }
};

struct FileTraits {
    static FILE* InvalidValue() {
        return nullptr;
    }
    static void Free(FILE* fp) {
#ifndef NDEBUG
        xassert2(fp != nullptr);
#endif
        fclose(fp);
    }
};
};  // namespace internal

template <typename T, typename Traits>
class UniqueResource {
 public:
    struct Data : public Traits {
        explicit Data(const T& t) : v(t) {
            xdebug2_if(v != Traits::InvalidValue(), TSF "%_ resource %_ acquired.", this, v);
        }
        T v;
    } data_;

 public:
    typedef T element_type;
    typedef Traits traits_type;

    UniqueResource() : data_(traits_type::InvalidValue()) {
    }
    explicit UniqueResource(const element_type& v) : data_(v) {
    }
    UniqueResource(UniqueResource&& rhs) : data_(rhs.release()) {
    }
    UniqueResource& operator=(UniqueResource&& rhs) {
        reset(rhs.release());
        return *this;
    }

    ~UniqueResource() {
        _Free();
    }

    const element_type& get() const {
        return data_.v;
    }
    element_type invalid_value() const {
        return traits_type::InvalidValue();
    }
    void reset(const element_type& v = traits_type::InvalidValue()) {
        if (data_.v != traits_type::InvalidValue() && data_.v == v) {
            xassert2(false, "can't reset self!!!!");
            return;
        }
        _Free();
        data_.v = v;
    }
    element_type release() {
        element_type old = data_.v;
        data_.v = traits_type::InvalidValue();
        return old;
    }
    bool is_valid() const {
        return data_.v != traits_type::InvalidValue();
    }
    void swap(UniqueResource& rhs) {
        std::swap(static_cast<Traits&>(data_), static_cast<Traits&>(rhs.data_));
        std::swap(data_.v, rhs.data_.v);
    }
    bool operator==(const UniqueResource& rhs) const {
        return data_.v == rhs.data_.v;
    }
    bool operator!=(const UniqueResource& rhs) const {
        return data_.v != rhs.data_.v;
    }

 private:
    void _Free() {
        if (data_.v != traits_type::InvalidValue()) {
            data_.Free(data_.v);
            xdebug2(TSF "%_ resource %_ released.", this, data_.v);
            data_.v = traits_type::InvalidValue();
        }
    }

 private:
    UniqueResource(const UniqueResource& rhs) = delete;
    UniqueResource& operator=(const UniqueResource& rhs) = delete;
};

using UniqueSocketResource = UniqueResource<SOCKET, internal::SocketTraits>;
using UniqueFDResource = UniqueResource<int, internal::FDTraits>;
using UniqueFileResource = UniqueResource<FILE*, internal::FileTraits>;
