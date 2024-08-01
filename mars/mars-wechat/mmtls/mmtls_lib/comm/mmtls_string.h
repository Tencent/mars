#pragma once
#include <openssl/crypto.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "mmtls_constants.h"

namespace mmtls {

class StringPiece {
 public:
    StringPiece() : buf_(NULL), size_(0) {
    }
    StringPiece(const char* buf, size_t size) : buf_(buf), size_(size) {
    }

    const char* c_str() const {
        return buf_;
    }
    const char* data() const {
        return buf_;
    }
    size_t size() const {
        return size_;
    }
    bool empty() const {
        return buf_ == NULL || size_ == 0;
    }

    bool operator==(const char* str) const {
        if (str == NULL)
            return empty();
        if (empty())
            return (str == NULL || strlen(str) == 0);
        if (::strlen(str) != size_)
            return false;
        return 0 == ::memcmp(buf_, str, size_);
    }

    bool operator==(const std::string& str) const {
        if (empty())
            return str.empty();
        if (str.size() != size_)
            return false;
        return 0 == ::memcmp(str.data(), buf_, size_);
    }

    bool operator==(const StringPiece& str) const {
        if (empty())
            return str.empty();
        if (str.empty())
            return empty();
        if (str.size() != size_)
            return false;
        return 0 == ::memcmp(str.data(), buf_, size_);
    }

 private:
    const char* buf_;
    size_t size_;
};

class String {
 public:
    String() : buf_(internal_buf_), size_(0), buf_size_(FIXED_STRING_UNIT_SIZE) {
        internal_buf_[0] = '\0';
    }
    String(const char* str) : buf_(internal_buf_), size_(0), buf_size_(FIXED_STRING_UNIT_SIZE) {
        append(str);
    }
    String(size_t size, char c) : buf_(internal_buf_), size_(0), buf_size_(FIXED_STRING_UNIT_SIZE) {
        append(size, c);
    }
    String(const char* buf, size_t size) : buf_(internal_buf_), size_(0), buf_size_(FIXED_STRING_UNIT_SIZE) {
        append(buf, size);
    }
    String(const String& str) : buf_(internal_buf_), size_(0), buf_size_(FIXED_STRING_UNIT_SIZE) {
        append(str);
    }
    explicit String(const std::string& str) : buf_(internal_buf_), size_(0), buf_size_(FIXED_STRING_UNIT_SIZE) {
        append(str.data(), str.size());
    }

    StringPiece string_piece() const {
        return StringPiece(buf_, size_);
    }

    std::string std_string() const {
        return std::string(buf_, size_);
    }

    ~String() {
        if (buf_ != NULL && buf_ != internal_buf_) {
            free(buf_);
        }
    }

    const char* c_str() const {
        return buf_;
    }
    const char* data() const {
        return buf_;
    }
    char* mutable_data() const {
        return buf_;
    }
    size_t size() const {
        return size_;
    }
    bool empty() const {
        return size_ == 0;
    }

    void clear() {
        size_ = 0;
    }

    void safe_clear() {
        OPENSSL_cleanse(buf_, size_);
        size_ = 0;
    }

    void move_from(String& str) {
        if (buf_ == str.buf_)
            return;

        reset();

        if (str.buf_ == str.internal_buf_) {
            ::memcpy(buf_, str.buf_, str.size_);
            size_ = str.size_;
        } else {
            buf_ = str.buf_;
            size_ = str.size_;
            buf_size_ = str.buf_size_;
        }
        str.buf_ = str.internal_buf_;
        str.size_ = 0;
        str.buf_size_ = FIXED_STRING_UNIT_SIZE;
        str.internal_buf_[0] = '\0';
    }

    void swap(String& str) {
        move_from(str);
    }

    void append(const std::string& str) {
        append(str.data(), str.size());
    }

    void append(const String& str) {
        append(str.buf_, str.size_);
    }

    void append(const char* str) {
        if (str == NULL)
            return;
        append(str, ::strlen(str));
    }

    void append(const char* buf, size_t size) {
        if (buf == NULL)
            return;
        resize(size_ + size);
        ::memcpy(buf_ + size_ - size, buf, size);
    }

    void append(size_t size, char c) {
        resize(size_ + size);
        ::memset(buf_ + size_ - size, c, size);
    }

    void assign(const char* buf, size_t size) {
        if (buf == NULL)
            return;
        clear();
        append(buf, size);
    }

    void assign(const char* str) {
        if (str == NULL)
            return;
        clear();
        append(str, ::strlen(str));
    }

    void assign(const std::string& str) {
        clear();
        append(str);
    }

    void assign(const String& str) {
        clear();
        append(str);
    }

    void replace(size_t pos, size_t len, const char* s, size_t n) {
        if (s == NULL)
            return;
        if (pos >= size_)
            return;
        size_t size = std::min(len, n);
        size = std::min(size, size_ - pos);
        memcpy(buf_ + pos, s, size);
    }

    void resize(size_t size) {
        reserve(size);
        size_ = size;
        buf_[size_] = '\0';
    }

    void resize(size_t size, char c) {
        resize(size);
        ::memset(buf_, c, size_);
    }

    void reserve(size_t size) {
        if (size >= buf_size_) {
            char* old_buf = buf_;
            size_t old_buf_size = buf_size_;
            buf_size_ = (size / FIXED_STRING_UNIT_SIZE + 1) * FIXED_STRING_UNIT_SIZE;
            buf_ = (char*)::malloc(buf_size_);
            if (buf_ == NULL) {
                buf_ = old_buf;
                buf_size_ = old_buf_size;
                return;
            }
            ::memcpy(buf_, old_buf, size_);
            if (old_buf != NULL && old_buf != internal_buf_) {
                free(old_buf), old_buf = NULL;
            }
        }
    }

    String substr(size_t pos) const {
        if (pos >= size_)
            return String();
        return String(buf_ + pos, size_ - pos);
    }

    String substr(size_t pos, size_t size) const {
        if (pos >= size_)
            return String();
        if (size > size_ - pos)
            size = size_ - pos;
        return String(buf_ + pos, size);
    }

    bool safe_eq(const char* str) const {
        if (str == NULL)
            return false;
        if (::strlen(str) != size_)
            return false;
        return 0 == CRYPTO_memcmp(buf_, str, size_);
    }

    bool safe_eq(const String& str) const {
        if (size_ != str.size_)
            return false;
        return 0 == CRYPTO_memcmp(buf_, str.buf_, size_);
    }

    bool safe_eq(const std::string& str) const {
        if (str.size() != size_)
            return false;
        return 0 == CRYPTO_memcmp(buf_, str.data(), size_);
    }

    bool operator==(const char* str) const {
        if (str == NULL)
            return false;
        if (::strlen(str) != size_)
            return false;
        return 0 == ::memcmp(buf_, str, size_);
    }

    bool operator==(const String& str) const {
        if (size_ != str.size_)
            return false;
        return 0 == ::memcmp(buf_, str.buf_, size_);
    }

    bool operator==(const std::string& str) const {
        if (str.size() != size_)
            return false;
        return 0 == ::memcmp(buf_, str.data(), size_);
    }

    bool operator!=(const char* str) const {
        if (str == NULL)
            return false;
        return !operator==(str);
    }

    bool operator!=(const String& str) const {
        return !operator==(str);
    }

    bool operator!=(const std::string& str) const {
        return !operator==(str);
    }

    String& operator+=(const char* str) {
        if (str == NULL)
            return (*this);
        append(str);
        return (*this);
    }

    String& operator+=(const String& str) {
        append(str);
        return (*this);
    }

    String& operator+=(const std::string& str) {
        append(str);
        return (*this);
    }

    String& operator=(const char* str) {
        if (str == NULL)
            return (*this);
        clear();
        append(str);
        return (*this);
    }

    String& operator=(const String& str) {
        clear();
        append(str);
        return (*this);
    }

    String& operator=(const std::string& str) {
        clear();
        append(str);
        return (*this);
    }

    char& operator[](size_t pos) const {
        static char null = '\0';
        null = '\0';
        if (pos >= size_)
            return null;
        return buf_[pos];
    }

    friend std::ostream& operator<<(std::ostream& os, const String& str) {
        os << std::string(str.data(), str.size());
        return os;
    }

 protected:
    void reset() {
        if (buf_ != NULL && buf_ != internal_buf_) {
            free(buf_);
        }
        buf_ = internal_buf_;
        size_ = 0;
        buf_size_ = FIXED_STRING_UNIT_SIZE;
        buf_[0] = '\0';
    }

 private:
    char internal_buf_[FIXED_STRING_UNIT_SIZE];
    char* buf_;
    size_t size_;
    size_t buf_size_;
};

String operator+(const String& lhs, const String& rhs);

bool operator==(const char* lhs, const String& rhs);

bool operator==(const std::string& lhs, const String& rhs);

bool operator==(const char* lhs, const StringPiece& rhs);

bool operator==(const std::string& lhs, const StringPiece& rhs);

}  // namespace mmtls
