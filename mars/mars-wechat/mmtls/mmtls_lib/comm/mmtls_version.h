#pragma once

#include "mmtls_data_reader.h"
#include "mmtls_data_writer.h"
#include "mmtls_types.h"

namespace mmtls {

/**
 * MMTLS protocol version
 */
class ProtocolVersion {
 public:
    enum VersionCode {
        TLS_VNONE = 0x0000,
        // The first version of mmtls.
        // TODO: change the enum name.
        TLS_V13 = 0xF103,
        // 支持cert region
        TLS_V14 = 0xF104,
    };

    static ProtocolVersion LatestVersion() {
        return ProtocolVersion(TLS_V14);
    }

    ProtocolVersion() : version_(TLS_VNONE) {
    }
    ProtocolVersion(uint16 code) : version_(code) {
    }

    ProtocolVersion(byte minor_ver, byte major_ver) : version_(static_cast<uint16>(major_ver) << 8 | minor_ver) {
    }

    int Serialize(DataWriter& writer) {
        writer.Write(version_);
        return OK;
    }

    int Deserialize(DataReader& reader) {
        return reader.Get(version_);
    }

    bool IsValid() const {
        return (version_ == TLS_V13) || (version_ == TLS_V14);
    }

    byte MinorVersion() const {
        return static_cast<byte>(version_ & 0XFF);
    }

    byte MajorVersion() const {
        return static_cast<byte>((version_ >> 8) & 0xFF);
    }

    uint16 version() const {
        return version_;
    }

    bool operator==(const ProtocolVersion& other) const {
        return version_ == other.version_;
    }

 private:
    uint16 version_;
};

}  // namespace mmtls
