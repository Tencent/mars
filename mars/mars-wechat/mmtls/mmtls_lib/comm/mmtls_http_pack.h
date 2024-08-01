#pragma once

#include <string>

#include "mmtls_data_pack.h"

namespace mmtls {

/**
 * �����л�ʱ������DeserializeReader���ڴ棬�������ά������������
 */
class HttpHandler : public DataPackInterface {
 public:
    HttpHandler();

    HttpHandler(const byte* url,
                size_t url_size,
                const byte* host,
                size_t host_size,
                const byte* mmpkg,
                size_t mmpkg_size);

    virtual ~HttpHandler();

    // accessor
    const byte* url() const {
        return url_;
    }
    /*const*/ size_t url_size() const {
        return url_size_;
    }

    const byte* host() const {
        return host_;
    }
    /*const*/ size_t host_size() const {
        return host_size_;
    }

    const byte* mmpkg() const {
        return mmpkg_;
    }
    /*const*/ size_t mmpkg_size() const {
        return mmpkg_size_;
    }

 protected:
    int DoSerialize(SerializeWriter& writer);

    int DoDeserialize(DeserializeReader& reader);

 private:
    const byte* url_;
    size_t url_size_;

    const byte* host_;
    size_t host_size_;

    const byte* mmpkg_;
    size_t mmpkg_size_;
};

}  // namespace mmtls
