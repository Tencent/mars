#pragma once

#include <string.h>
#include <sys/time.h>

#include <sstream>
#include <vector>

#include "mmtls_log.h"
#include "mmtls_string.h"
#include "mmtls_types.h"
#include "stdlib.h"

#ifdef WIN32
#ifndef unlikely
#define unlikely(x) x
#endif

#ifndef likely
#define likely(x) x
#endif
#else
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#endif

namespace mmtls {

template <typename T>
inline T rotate_left(T input, size_t rot) {
    if (rot == 0) {
        return input;
    }
    return static_cast<T>((input << rot) | (input >> (8 * sizeof(T) - rot)));
}

template <typename T>
inline T rotate_right(T input, size_t rot) {
    if (rot == 0) {
        return input;
    }
    return static_cast<T>((input >> rot) | (input << (8 * sizeof(T) - rot)));
}
inline byte reverse_bytes(byte val) {
    return val;
}

inline uint16 reverse_bytes(uint16 val) {
    return rotate_left(val, 8);
}

inline uint32 reverse_bytes(uint32 val) {
    return (rotate_right(val, 8) & 0xFF00FF00) | (rotate_left(val, 8) & 0x00FF00FF);
}

inline uint64 reverse_bytes(uint64 val) {
    uint32 hi = static_cast<uint32>(val >> 32);
    uint32 lo = static_cast<uint32>(val);
    return ((static_cast<uint64>(reverse_bytes(lo)) << 32) | reverse_bytes(hi));
}

mmtls::String StrToHex(const mmtls::String& str);

mmtls::String SafeStrToHex(const mmtls::String& str);

int64_t DiffTimeUs(timeval* begin, timeval* end);

/*
 *@note 返回时间UTC的毫秒粒度
 */
uint64 NowUTime();

uint32 NowTime();

namespace {

template <typename E_F>
void _Print_Value(std::stringstream& ss, const E_F& v) {
    ss << v;
    return;
};

#define _MAX_LOG_LINE 1024

#if 0
#define _Print_Log(ef, es, expr1, expr2, buffer, ev)

#else

#ifdef __UNITEST_BUILD__

#define _Print_Log(ef, es, expr1, expr2, buffer, ev)                       \
    if (!mmtls::g_Log.IsDiscard(ILog::MMTLS_LOG_ERR)) {                    \
        buffer = (char*)malloc(_MAX_LOG_LINE);                             \
        std::stringstream ss;                                              \
        ss << __func__;                                                    \
        ss << "[ " << expr1 << " ] " << ev << " [ " << expr2 << " ] --> "; \
        ss << "[ ";                                                        \
        _Print_Value(ss, ef);                                              \
        ss << " ] " << ev << " [ ";                                        \
        _Print_Value(ss, es);                                              \
        ss << " ]";                                                        \
        snprintf(buffer, _MAX_LOG_LINE, "%s", ss.str().c_str());           \
    }

#else

#define _Print_Log(ef, es, expr1, expr2, buffer, ev)             \
    if (!mmtls::g_Log.IsDiscard(ILog::MMTLS_LOG_ERR)) {          \
        buffer = (char*)malloc(_MAX_LOG_LINE);                   \
        std::stringstream ss;                                    \
        ss << __func__;                                          \
        ss << "[ ";                                              \
        _Print_Value(ss, ef);                                    \
        ss << " ] " << ev << " [ ";                              \
        _Print_Value(ss, es);                                    \
        ss << " ]";                                              \
        snprintf(buffer, _MAX_LOG_LINE, "%s", ss.str().c_str()); \
    }

#endif

#endif

template <typename E_F, typename E_S>
inline bool CHECK_EQ(const E_F& ef, const E_S& es, const char* expr1, const char* expr2, char*& buffer) {
    if (likely((ef) == (es))) {
        return true;
    }
    _Print_Log(ef, es, expr1, expr2, buffer, "==");
    return false;
}

template <typename E_F, typename E_S>
inline bool CHECK_NE(const E_F& ef, const E_S& es, const char* expr1, const char* expr2, char*& buffer) {
    if (likely((ef) != (es))) {
        return true;
    }
    _Print_Log(ef, es, expr1, expr2, buffer, "!=");
    return false;
}

template <typename E_F, typename E_S>
inline bool CHECK_LE(const E_F& ef, const E_S& es, const char* expr1, const char* expr2, char*& buffer) {
    if (likely((ef) <= (es))) {
        return true;
    }
    _Print_Log(ef, es, expr1, expr2, buffer, "<=");
    return false;
}

template <typename E_F, typename E_S>
inline bool CHECK_LT(const E_F& ef, const E_S& es, const char* expr1, const char* expr2, char*& buffer) {
    if (likely(((ef) < (es)))) {
        return true;
    }
    _Print_Log(ef, es, expr1, expr2, buffer, "<");
    return false;
}

template <typename E_F, typename E_S>
inline bool CHECK_GT(const E_F& ef, const E_S& es, const char* expr1, const char* expr2, char*& buffer) {
    if (likely((ef) > (es))) {
        return true;
    }
    _Print_Log(ef, es, expr1, expr2, buffer, ">");
    return false;
}

template <typename E_F, typename E_S>
inline bool CHECK_GE(const E_F& ef, const E_S& es, const char* expr1, const char* expr2, char*& buffer) {
    if (likely((ef) >= (es))) {
        return true;
    }
    _Print_Log(ef, es, expr1, expr2, buffer, ">=");
    return false;
}
}  // namespace

#define MMTLS_CHECK_TRUE(expr, ret, format, ...)                            \
    do {                                                                    \
        if (unlikely(!(expr))) {                                            \
            MMTLSLOG_ERR("CHECK_TRUE [%s] " #format, #expr, ##__VA_ARGS__); \
            return (ret);                                                   \
        }                                                                   \
    } while (0)

#define MMTLS_CHECK_FALSE(expr, ret, format, ...)                            \
    do {                                                                     \
        if (unlikely((expr))) {                                              \
            MMTLSLOG_ERR("CHECK_FALSE [%s] " #format, #expr, ##__VA_ARGS__); \
            return (ret);                                                    \
        }                                                                    \
    } while (0)

#define MMTLS_CHECK_EQ(expr1, expr2, ret, format, ...)                       \
    do {                                                                     \
        char* buffer = NULL;                                                 \
        if (unlikely(!CHECK_EQ((expr1), (expr2), #expr1, #expr2, buffer))) { \
            MMTLSLOG_ERR("%s " #format, buffer, ##__VA_ARGS__);              \
            free(buffer);                                                    \
            return (ret);                                                    \
        }                                                                    \
    } while (0)

#define MMTLS_CHECK_NE(expr1, expr2, ret, format, ...)                       \
    do {                                                                     \
        char* buffer = NULL;                                                 \
        if (unlikely(!CHECK_NE((expr1), (expr2), #expr1, #expr2, buffer))) { \
            MMTLSLOG_ERR("%s " #format, buffer, ##__VA_ARGS__);              \
            free(buffer);                                                    \
            return (ret);                                                    \
        }                                                                    \
    } while (0)

#define MMTLS_CHECK_GT(expr1, expr2, ret, format, ...)                       \
    do {                                                                     \
        char* buffer = NULL;                                                 \
        if (unlikely(!CHECK_GT((expr1), (expr2), #expr1, #expr2, buffer))) { \
            MMTLSLOG_ERR("%s " #format, buffer, ##__VA_ARGS__);              \
            free(buffer);                                                    \
            return (ret);                                                    \
        }                                                                    \
    } while (0)

#define MMTLS_CHECK_GE(expr1, expr2, ret, format, ...)                       \
    do {                                                                     \
        char* buffer = NULL;                                                 \
        if (unlikely(!CHECK_GE((expr1), (expr2), #expr1, #expr2, buffer))) { \
            MMTLSLOG_ERR("%s " #format, buffer, ##__VA_ARGS__);              \
            free(buffer);                                                    \
            return (ret);                                                    \
        }                                                                    \
    } while (0)

#define MMTLS_CHECK_LT(expr1, expr2, ret, format, ...)                       \
    do {                                                                     \
        char* buffer = NULL;                                                 \
        if (unlikely(!CHECK_LT((expr1), (expr2), #expr1, #expr2, buffer))) { \
            MMTLSLOG_ERR("%s " #format, buffer, ##__VA_ARGS__);              \
            free(buffer);                                                    \
            return (ret);                                                    \
        }                                                                    \
    } while (0)

#define MMTLS_CHECK_LE(expr1, expr2, ret, format, ...)                       \
    do {                                                                     \
        char* buffer = NULL;                                                 \
        if (unlikely(!CHECK_LE((expr1), (expr2), #expr1, #expr2, buffer))) { \
            MMTLSLOG_ERR("%s " #format, buffer, ##__VA_ARGS__);              \
            free(buffer);                                                    \
            return (ret);                                                    \
        }                                                                    \
    } while (0)

template <typename T>
class AutoPointHolder {
 public:
    AutoPointHolder(T*& t) : t_(t), auto_delete_(true) {
    }
    ~AutoPointHolder() {
        if (t_ && auto_delete_) {
            delete t_;
            t_ = NULL;
        }
    }
    void Disable() {
        auto_delete_ = false;
    }
    T* operator->() {
        return t_;
    }

 private:
    T*& t_;
    bool auto_delete_;
};

};  // namespace mmtls
