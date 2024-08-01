#include "mmtls_heartbeat_messages.h"

#include "mmtls_data_pack.h"
#include "mmtls_utils.h"

namespace mmtls {

Heartbeat::Heartbeat() : heartbeat_type_(0), payload_(""), padding_("") {
}

Heartbeat::Heartbeat(const HeartbeatType heartbeat_type, const mmtls::String& payload, const mmtls::String& padding)
: heartbeat_type_(heartbeat_type), payload_(payload), padding_(padding) {
}

Heartbeat::~Heartbeat() {
}

int Heartbeat::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_EQ(padding_.size(), FIXED_RANDOM_SIZE, ERR_ILLEGAL_PARAM, "padding size");
    MMTLS_CHECK_LE(payload_.size(), MAX_HEARTBEAT_PAYLOAD_SIZE, ERR_ILLEGAL_PARAM, "too large payload size");

    writer.Write(heartbeat_type_);
    writer.Write((uint16)payload_.size());
    writer.Write(payload_);
    writer.Write(padding_.c_str(), FIXED_RANDOM_SIZE);

    return OK;
}

int Heartbeat::DoDeserialize(DeserializeReader& reader) {
    int ret = reader.Get(heartbeat_type_);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get message type");

    uint16 payload_size = 0;
    ret = reader.Get(payload_size);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get payload size");

    ret = reader.GetString(payload_, payload_size);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get payload");

    ret = reader.GetString(padding_, FIXED_RANDOM_SIZE);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get padding");

    return OK;
}

}  // namespace mmtls
