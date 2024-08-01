#pragma once

#include <string>

#include "mmtls_constants.h"
#include "mmtls_types.h"
#include "mmtls_utils.h"

namespace mmtls {
class DataWriter {
 public:
    DataWriter() : buf_(&internal_buf_) {
    }
    DataWriter(mmtls::String& buf) : buf_(&buf) {
    }
    void Reset(mmtls::String* buf) {
        buf_ = buf;
    }

    size_t Tell() const {
        return buf_->size();
    }

    size_t Size() const {
        return buf_->size();
    }

    template <typename T>
    int Replace(size_t pos, const T& val) {
        if (buf_->size() <= pos + sizeof(val)) {
            return ERR_ILLEGAL_PARAM;
        }
        if (__BYTE_ORDER == __BIG_ENDIAN || sizeof(val) <= 1) {
            buf_->replace(pos, sizeof(val), (const char*)&val, sizeof(val));
            return OK;
        }

        T new_val = mmtls::reverse_bytes(val);
        buf_->replace(pos, sizeof(new_val), (const char*)&new_val, sizeof(new_val));
        return OK;
    }

    int Replace(size_t pos, const void* buf, size_t size) {
        if ((!buf) || 0 == size || buf_->size() <= pos + size) {
            return ERR_ILLEGAL_PARAM;
        }
        buf_->replace(pos, size, (const char*)buf, size);
        // MMTLSLOG_ERR("repleace at pos %zu size %zu fail",pos,size);
        return OK;
    }

    /*
     * @note clear alreay existing bytes.
     */
    void Clear() {
        buf_->clear();
    }

    /*
     *@note write binary data using network order,to buff
     *
     */
    void Write(const void* buf, size_t size) {
        buf_->append((const char*)buf, size);
    }

    /*
     *@note write binary data using network order,to buff
     *
     */
    void Write(const mmtls::String& buf) {
        buf_->append(buf);
    }

    /**
     *@note write host data to network buffer
     *@param val : type T must define in mmtls_types.h
     */
    template <typename T>
    void Write(const T& val) {
        if (__BYTE_ORDER == __BIG_ENDIAN || sizeof(val) <= 1) {
            buf_->append((const char*)&val, sizeof(val));
            return;
        }

        T new_val = mmtls::reverse_bytes(val);
        buf_->append((const char*)&new_val, sizeof(new_val));
    }

    /**
     *@note write host data to network buffer
     *@param val : type T must be uint32 or uint16
     */
    template <typename T>
    void WriteVarintT(T val) {
        static const unsigned int B = 128;
        static const unsigned int MASK = 0xFF;
        do {
            if (val >= B) {
                buf_->append(1, (byte)((B | val) & MASK));
            } else {
                buf_->append(1, (byte)(val & MASK));
            }
            val >>= 7;
        } while (val);
    }

    const mmtls::String& Buffer() const {
        return *buf_;
    }

    mmtls::String& MutableBuffer() const {
        return *buf_;
    }

 private:
    mmtls::String* buf_;
    mmtls::String internal_buf_;
};
};  // namespace mmtls
