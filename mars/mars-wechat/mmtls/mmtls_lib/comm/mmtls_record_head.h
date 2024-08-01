#pragma once

#include <string>

#include "mmtls_constants.h"
#include "mmtls_data_reader.h"
#include "mmtls_data_writer.h"
#include "mmtls_version.h"

namespace mmtls {

// RecordHead is the data unit transit between peers. It encrypt the payload to be transisted and append some header
// informations. This class only contain record header information.
//
// See https://tlswg.github.io/tls13-spec/#record-payload-protection
/*
 * serialized record layout:
 * type_[8bit],version_[16bit],payloadsize[16bit],payload_[payloadsize]
 */
class RecordHead {
 public:
    RecordHead() : type_(RECORD_NONE), version_(0), payload_size_(0) {
    }

    /*
     * @note read record head from reader
     *@return
     * 	     return need length if reader data not enogh.
     * 	     succ: 0
     * 	     <0 : error occurs
     */
    int Deserialize(DataReader& reader);

    int Assign(RecordType type, const ProtocolVersion& version, size_t payload_size);

    void SetType(RecordType type) {
        type_ = type;
    }
    RecordType GetType() const {
        return type_;
    }

    void SetProtocolVersion(const ProtocolVersion& version) {
        version_ = version;
    }
    ProtocolVersion GetProtocolVersion() const {
        return version_;
    }

    void SetPayloadSize(size_t payload_size) {
        payload_size_ = payload_size;
    }
    size_t GetPayloadSize() const {
        return payload_size_;
    }

    int Serialize(DataWriter& writer);

    void SetInvalid() {
        type_ = RECORD_NONE;
    }
    bool IsValid() const {
        return type_ == RECORD_ALERT || type_ == RECORD_HANDSHAKE || type_ == RECORD_APPLICATION_DATA
               || type_ == RECORD_EARLY_HANDSHAKE || type_ == RECORD_HEARTBEAT;
    }

 protected:
    bool operator==(const RecordHead& record) const;

 private:
    // The record type.
    RecordType type_;
    // The record protocol version.
    ProtocolVersion version_;
    // The record payload size.
    size_t payload_size_;
};

}  // namespace mmtls
