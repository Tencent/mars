#pragma once

#include "mmtls_policy.h"

namespace mmtls {

class ClientPolicy : public Policy {
 public:
    ClientPolicy() {
    }
    virtual ~ClientPolicy() {
    }

    virtual bool AcceptProtocolVersion(const ProtocolVersion& version) {
        return version.version() == ProtocolVersion::TLS_V14;
    }
    size_t MaxAppDataSize() {
        return 4 * 1024 * 1024;
    }
    static ClientPolicy& GetInstance();
};

}  // namespace mmtls
