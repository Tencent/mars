#pragma once
#include <vector>

#include "mmtls_ciphersuite.h"
#include "mmtls_version.h"

namespace mmtls {

/**
 * TLS Policy Base Class
 * Inherit and overload as desired to suit local policy concerns
 */
class Policy {
 public:
    Policy() {
    }
    virtual ~Policy() {
    }
    virtual ProtocolVersion LatestSupportedProtocolVersion() const {
        return ProtocolVersion::LatestVersion();
    }

    virtual bool AcceptProtocolVersion(const ProtocolVersion& version) {
        return version.IsValid();
    }

    virtual size_t MaxRecordPlaintextSize() {
        return 32 * 1024;
    }

    virtual size_t MaxAppDataSize() {
        return 128 * 1024;
    }
};

}  // namespace mmtls
