#include "mmtls_record_head.h"

#include "mmtls_data_writer.h"
#include "mmtls_log.h"
#include "mmtls_utils.h"

using namespace mmtls;

int RecordHead::Deserialize(DataReader& reader) {
    if (reader.Remaining() < FIXED_RECORD_HEAD_SIZE) {
        return (int)(FIXED_RECORD_HEAD_SIZE - reader.Remaining());
    }
    MMTLS_CHECK_EQ(OK, reader.Get(type_), ERR_DECODE_ERROR, "get type fail");
    uint16 version = 0;
    MMTLS_CHECK_EQ(OK, reader.Get(version), ERR_DECODE_ERROR, "get version fail");
    version_ = version;

    uint16 payload_size = 0;
    MMTLS_CHECK_EQ(OK, reader.Get(payload_size), ERR_DECODE_ERROR, "get payload size fail");
    payload_size_ = payload_size;
    MMTLS_CHECK_LE(payload_size_, MAX_CIPHERTEXT_SIZE, ERR_LENGTH_EXCEED_LIMIT, "record payload size exceed limit");

    MMTLS_CHECK_TRUE(IsValid(), ERR_DECODE_ERROR, "record head invalid");

    return OK;
}

int RecordHead::Assign(RecordType type, const ProtocolVersion& version, size_t payload_size) {
    MMTLS_CHECK_NE(type, RECORD_NONE, ERR_ILLEGAL_PARAM, "record type is invliad");
    MMTLS_CHECK_NE(version.version(), ProtocolVersion::TLS_VNONE, ERR_ILLEGAL_PARAM, "record version is invalid");
    MMTLS_CHECK_LE(payload_size, MAX_CIPHERTEXT_SIZE, ERR_LENGTH_EXCEED_LIMIT, "record payload size exceed limit");

    type_ = type;
    version_ = version;
    payload_size_ = payload_size;

    MMTLS_CHECK_TRUE(IsValid(), ERR_ILLEGAL_PARAM, "record head invalid");

    return OK;
}

int RecordHead::Serialize(DataWriter& writer) {
    MMTLS_CHECK_TRUE(IsValid(), ERR_ENCODE_ERROR, "record head invalid");
    MMTLS_CHECK_LE(payload_size_, MAX_CIPHERTEXT_SIZE, ERR_LENGTH_EXCEED_LIMIT, "record payload size exceed limit");

    writer.Write(static_cast<byte>(type_));
    writer.Write(version_.version());
    writer.Write(static_cast<uint16>(payload_size_));
    return OK;
}

bool RecordHead::operator==(const RecordHead& record) const {
    return (record.GetProtocolVersion().version() == version_.version() && record.GetType() == type_
            && record.GetPayloadSize() == payload_size_);
}
