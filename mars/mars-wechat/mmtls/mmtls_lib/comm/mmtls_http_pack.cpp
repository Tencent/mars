#include "mmtls_http_pack.h"

#include "mmtls_data_pack.h"
#include "mmtls_data_reader.h"
#include "mmtls_data_writer.h"

namespace mmtls {

HttpHandler::HttpHandler() : url_(NULL), url_size_(0), host_(NULL), host_size_(0), mmpkg_(NULL), mmpkg_size_(0) {
}

HttpHandler::HttpHandler(const byte* url,
                         size_t url_size,
                         const byte* host,
                         size_t host_size,
                         const byte* mmpkg,
                         size_t mmpkg_size)
: url_(url), url_size_(url_size), host_(host), host_size_(host_size), mmpkg_(mmpkg), mmpkg_size_(mmpkg_size) {
}

HttpHandler::~HttpHandler() {
}

int HttpHandler::DoSerialize(SerializeWriter& writer) {
    // set url
    writer.Write((uint16)url_size_);
    writer.Write(url_, url_size_);

    // set host
    writer.Write((uint16)host_size_);
    writer.Write(host_, host_size_);

    // set mmpkg
    writer.Write((uint32)mmpkg_size_);
    writer.Write(mmpkg_, mmpkg_size_);

    return OK;
}

int HttpHandler::DoDeserialize(DeserializeReader& reader) {
    int ret = 0;
    uint16 len = 0;
    const byte* buff = NULL;

    // get url
    ret = reader.Get(len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get url len failed");

    buff = reader.GetString(len);
    MMTLS_CHECK_TRUE(buff, ERR_NO_ENOUGH_DATA, "get url buffer failed, url len %u", len);

    url_ = buff;
    url_size_ = len;

    // get host
    ret = reader.Get(len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get host len failed");

    buff = reader.GetString(len);
    MMTLS_CHECK_TRUE(buff, ERR_NO_ENOUGH_DATA, "get host buffer failed, host len %u", len);

    host_ = buff;
    host_size_ = len;

    // get mmpkg
    uint32 mmpkg_len = 0;
    ret = reader.Get(mmpkg_len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get mmpkg len failed");

    buff = reader.GetString(mmpkg_len);
    MMTLS_CHECK_TRUE(buff, ERR_NO_ENOUGH_DATA, "get mmpkg buffer failed, mmpkg len %u", mmpkg_len);

    mmpkg_ = buff;
    mmpkg_size_ = mmpkg_len;

    return OK;
}

}  // namespace mmtls
