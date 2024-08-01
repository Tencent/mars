#pragma once

#include <memory>
#include <string>

#include "mmtls_constants.h"
#include "mmtls_types.h"
#include "mmtls_utils.h"

namespace mmtls {

/*
 *@note Read Data From Network flow
 */
class DataReader {
#define Reader_CHECK_SIZE(n) \
    MMTLS_CHECK_GE(size_, offset_ + n, ERR_NO_ENOUGH_DATA, "size %zu less offset %zu n %u", size_, offset_, n);

 public:
    DataReader(const mmtls::String& buf) : buf_(buf.data()), size_(buf.size()), offset_(0) {
    }

    DataReader(const char* buf, size_t size) : buf_(buf), size_(size), offset_(0) {
    }

    virtual ~DataReader() {
        ;
    }

    size_t Remaining() const {
        return size_ - offset_;
    }

    size_t Tell() const {
        return offset_;
    }

    const char* Begin() const {
        return buf_;
    }

    bool HasRemaining() const {
        return (Remaining() > 0);
    }

    mmtls::String GetRemaining() {
        if (!HasRemaining()) {
            return mmtls::String();
        }
        mmtls::String val(buf_ + offset_, size_ - offset_);
        offset_ = size_;
        return val;
    }

    int DiscardNext(size_t bytes) {
        MMTLS_CHECK_LE(bytes,
                       Remaining(),
                       ERR_NO_ENOUGH_DATA,
                       "discard bytes %zu greater than remain %zu",
                       bytes,
                       Remaining());
        offset_ += bytes;
        return OK;
    }

    /**
     * @note read network buffer to host type data
     * @param val : type T must define in mmtls_types.h
     * @param move_offset : true then move the offset forward
     * @return 0 :succ,get val
     * 		 else : fail
     */
    template <typename T>
    int Get(T& val, bool move_offset = true) {
        Reader_CHECK_SIZE((unsigned int)sizeof(T));
        if (__BYTE_ORDER == __LITTLE_ENDIAN && sizeof(T) > 1) {
            memcpy(&val, buf_ + offset_, sizeof(T));
            val = mmtls::reverse_bytes(val);
        } else {
            memcpy(&val, buf_ + offset_, sizeof(T));
        }
        if (move_offset) {
            offset_ += sizeof(T);
        }
        return OK;
    }

    /**
     * @note read network buffer to host type data
     * @param val : type T must be uint32 or uint16
     */
    template <typename T>
    int GetVarintT(T& val, bool move_offset = true) {
        static const unsigned int B = 128;

        val = 0;
        size_t next_offset = offset_;
        for (size_t pow = 0; next_offset < size_; pow += 7) {
            size_t index = next_offset++;
            T v = *(unsigned char*)(buf_ + index);
            if (buf_[index] & B) {
                // T at most encoded sizeof(T)+1 bytes.
                if (next_offset - offset_ > sizeof(T) + 1)
                    return ERR_DECODE_ERROR;
                val |= (v ^ B) << pow;
            } else {
                val |= v << pow;
                if (move_offset) {
                    offset_ = next_offset;
                }
                return OK;
            }
        }
        return ERR_DECODE_ERROR;
    }

    /*@note get string from data
     *@param val : output string
     *@param size : need size
     *
     * @return 0 :succ,get data with need size
     * 		 else : fail
     */
    int GetString(mmtls::String& val, size_t size) {
        Reader_CHECK_SIZE((unsigned int)size);
        val.assign(buf_ + offset_, size);
        offset_ += size;
        return OK;
    }

    /*@note get string from data
     *@param size : need size
     *
     * @return buf addr :succ,get buff addr
     * 		 NULL : fail
     */
    const byte* GetString(size_t size) {
        if (size_ < offset_ + size) {
            return NULL;
        }
        offset_ += size;
        return (const byte*)(buf_ + offset_ - size);
    }

    DataReader* SubReader() const {
        return new DataReader(buf_ + offset_, Remaining());
    }

 private:
    const char* buf_;
    size_t size_;
    size_t offset_;
};
}  // namespace mmtls
