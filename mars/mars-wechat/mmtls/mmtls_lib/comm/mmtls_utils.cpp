#include "mmtls_utils.h"

#include <sstream>

namespace mmtls {

int64_t DiffTimeUs(timeval* begin, timeval* end) {
    int64_t diff_time = (end->tv_sec - begin->tv_sec) * 1000000L + end->tv_usec - begin->tv_usec;

    if (diff_time < 0)
        diff_time = 0;
    return diff_time;
}

uint64 NowUTime() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ULL + tv.tv_usec / 1000;
}

uint32 NowTime() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32)tv.tv_sec;
    // return time(NULL);
}

mmtls::String StrToHex(const mmtls::String& str) {
    const char* sSrc = str.data();
    size_t len = str.size();
    mmtls::String res;
    res.reserve(len * 2 + 1);
    char hex[3] = {0};
    for (size_t i = 0; i < len; ++i) {
        snprintf(hex, sizeof(hex), "%02x", (unsigned char)(sSrc[i]));
        res.append(hex, 2);
    }
    return res;
}

mmtls::String SafeStrToHex(const mmtls::String& str) {
    mmtls::String safe_hex = StrToHex(str);
    if (safe_hex.size() > 32) {
        safe_hex = safe_hex.substr(0, 16) + safe_hex.substr(safe_hex.size() - 16);
    }

    size_t safe_hex_size = safe_hex.size();
    size_t begin = safe_hex_size / 4;
    size_t end = safe_hex_size * 3 / 4;
    for (size_t i = begin; i < end; ++i) {
        safe_hex[i] = '*';
    }

    return safe_hex;
}

}  // namespace mmtls
