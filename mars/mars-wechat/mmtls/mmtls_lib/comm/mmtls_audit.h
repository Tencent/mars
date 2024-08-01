#pragma once

#include <stdarg.h>

#include "mmtls_constants.h"
#include "mmtls_utils.h"

#define RETURN_CASE_BIT(value, bit) \
    case (value): {                 \
        return 1 << (bit);          \
    }
#define RETURN_DEFAULT_BIT(bit) \
    default: {                  \
        return 1 << (bit);      \
    }

#define SET_FLAGS(type, flags, count)                  \
    va_list args;                                      \
    va_start(args, count);                             \
    do {                                               \
        (flags) |= (FlagBit)((type)va_arg(args, int)); \
    } while (--(count) > 0);                           \
    va_end(args);
#define SET_FLAG(flags, val) (flags) |= FlagBit(val)

namespace mmtls {
template <typename T, typename FlagType>
class Audit {
 public:
    Audit() : flags_(0) {
    }
    Audit(uint32 count, ...) {
        flags_ = 0;
        SET_FLAGS(T, flags_, count);
    }
    Audit(const T& val) {
        flags_ = 0;
        SET_FLAG(flags_, val);
    }
    virtual ~Audit() {
    }

    void set_flag(const T& val) {
        SET_FLAG(flags_, val);
    }
    void set_flags(uint32 count, ...) {
        SET_FLAGS(T, flags_, count);
    }

    FlagType flags() const {
        return flags_;
    }

    int CheckEqual(const Audit& check_audit) {
        MMTLS_CHECK_EQ(flags_,
                       check_audit.flags(),
                       ERR_AUDIT_CHECK_FAIL,
                       "check equal fail.flags 0x%x checkflags 0x%x",
                       flags_,
                       check_audit.flags());
        return OK;
    }
    int CheckCover(const Audit& check_audit) {
#ifdef __LP64__
        MMTLS_CHECK_EQ((flags_ & check_audit.flags()),
                       flags_,
                       ERR_AUDIT_CHECK_FAIL,
                       "heck cover fail.flags 0x%lu checkflags 0x%lu",
                       flags_,
                       check_audit.flags());
#else
        MMTLS_CHECK_EQ((flags_ & check_audit.flags()),
                       flags_,
                       ERR_AUDIT_CHECK_FAIL,
                       "heck cover fail.flags 0x%llx checkflags 0x%llx",
                       flags_,
                       check_audit.flags());
#endif
        return OK;
    }

    int CheckHas(const Audit& check_audit) {
#ifdef __LP64__
        MMTLS_CHECK_EQ((flags_ & check_audit.flags()),
                       check_audit.flags(),
                       ERR_AUDIT_CHECK_FAIL,
                       "check has fail.flags 0x%lu checkflags 0x%lu",
                       flags_,
                       check_audit.flags());
#else
        MMTLS_CHECK_EQ((flags_ & check_audit.flags()),
                       check_audit.flags(),
                       ERR_AUDIT_CHECK_FAIL,
                       "check has fail.flags 0x%llx checkflags 0x%llx",
                       flags_,
                       check_audit.flags());
#endif
        return OK;
    }

 protected:
    virtual FlagType FlagBit(T val) {
        MMTLSLOG_DEBUG("flag bit val 0x%x 0x%x", val, (1 << val));
        return (1 << val);
    }

 protected:
    FlagType flags_;
};

class StageAudit : public Audit<HandShakeStage, uint32> {
 public:
    StageAudit() {
    }
    StageAudit(HandShakeStage val) : Audit(val) {
    }
    StageAudit(uint32 count, ...) {
        SET_FLAGS(HandShakeStage, flags_, count);
    }
    virtual ~StageAudit() {
    }

 protected:
    virtual uint32 FlagBit(HandShakeStage stage) {
        switch (stage) {
            RETURN_CASE_BIT(STAGE_INIT, 1)
            // client stages
            RETURN_CASE_BIT(STAGE_SEND_CLIENT_HELLO, 2)
            RETURN_CASE_BIT(STAGE_SEND_ENCRYPTED_EXTENSIONS, 3)
            RETURN_CASE_BIT(STAGE_SEND_EARLY_APP_DATA, 4)
            RETURN_CASE_BIT(STAGE_RECV_SERVER_HELLO, 5)
            RETURN_CASE_BIT(STAGE_RECV_CERTIFICATE_VERIFY, 6)
            RETURN_CASE_BIT(STAGE_RECV_NEW_SESSION_TICKET, 7)
            RETURN_CASE_BIT(STAGE_RECV_SERVER_FINISHED, 8)
            RETURN_CASE_BIT(STAGE_RECV_EARLY_APP_DATA, 9)
            RETURN_CASE_BIT(STAGE_SEND_CLIENT_FINISHED, 10)

            // server stages
            RETURN_CASE_BIT(STAGE_RECV_CLIENT_HELLO, 11)
            RETURN_CASE_BIT(STAGE_RECV_ENCRYPTED_EXTENSIONS, 12)
            RETURN_CASE_BIT(STAGE_SEND_SERVER_HELLO, 13)
            RETURN_CASE_BIT(STAGE_SEND_CERTIFICATE_VERIFY, 14)
            RETURN_CASE_BIT(STAGE_SEND_NEW_SESSION_TICKET, 15)
            RETURN_CASE_BIT(STAGE_RECV_CLIENT_EARLY_DATA, 16)
            RETURN_CASE_BIT(STAGE_SEND_SERVER_EARLY_DATA, 17)
            RETURN_CASE_BIT(STAGE_SEND_SERVER_FINISHED, 18)
            RETURN_CASE_BIT(STAGE_RECV_CLIENT_FINISHED, 19)

            RETURN_CASE_BIT(STAGE_COMPLETED, 20)
            RETURN_CASE_BIT(STAGE_CLOSED, 21)

            RETURN_DEFAULT_BIT(31);
        }
    }
};

}  // namespace mmtls
