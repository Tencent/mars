//
// Created by garry on 2023/2/20.
//

#ifndef MARS_COMM_TL_VARINT_V_H_
#define MARS_COMM_TL_VARINT_V_H_

#include <stddef.h>
#include <stdint.h>
#include <string>

namespace mars {
namespace comm {

template<typename T>
size_t varint_encode(T value, char *output) {
    size_t outputSize = 0;
    // While more than 7 bits of data are left, occupy the last output byte
    //  and set the next byte flag
    while (value > 127) {
        //|128: Set the next byte flag
        output[outputSize] = ((uint8_t)(value & 127)) | 128;
        // Remove the seven bits we just wrote
        value >>= 7;
        outputSize++;
    }
    output[outputSize++] = ((uint8_t)value) & 127;
    return outputSize;
}

class TLVarIntV {
 private:
    enum {
        kVarIntTypeChar = 0,
        kVarIntTypeIntNum,
        kVarIntTypeString,
    };
 public:
    TLVarIntV(const std::string &_value) {
        char buffer[8] = {0};
        size_t size = _value.size();
        size_t len = varint_encode((uint16_t)kVarIntTypeString, buffer);
        len += varint_encode(size, buffer + len);
        value_.append(buffer, len);
        value_ += _value;
    }

    TLVarIntV(const char* _value, size_t _max_value_len = 16 * 1024) {
        char buffer[8] = {0};
        size_t size = strnlen(_value, _max_value_len);
        size_t len = varint_encode((uint16_t)kVarIntTypeString, buffer);
        len += varint_encode(size, buffer + len);
        value_.append(buffer, len);
        value_.append(_value, size);
    }

    TLVarIntV(int _value) {
        char buffer[8] = {0};
        size_t len = varint_encode((uint16_t)kVarIntTypeIntNum, buffer);
        len += varint_encode(_value, buffer + len);
        value_.append(buffer, len);
    }

    const std::string& str() const {
        return value_;
    }

 private:
    std::string value_;
};


}
}

#endif //MARS_COMM_TL_VARINT_V_H_
