#include "mmtls_handshake_messages.h"

#include <memory>

#include "mmtls_data_pack.h"
#include "mmtls_utils.h"

namespace mmtls {
int HandShakeMessage::DetectHandShakeType(const DataReader& reader, HandShakeType& type) {
    return DataPackInterface::DetectPackNumber(type, reader, 0);
}

// ClientHello implemetation

int ClientHello::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_LE(ciphersuites_.size(), MAX_CIPHERSUITE_SIZE, ERR_ILLEGAL_PARAM, "cipher suite counter");
    MMTLS_CHECK_EQ(FIXED_RANDOM_SIZE, random_.size(), ERR_ILLEGAL_PARAM, "client random size");

    writer.Write(GetType());
    writer.Write(version_.MinorVersion());
    writer.Write(version_.MajorVersion());
    writer.Write((byte)ciphersuites_.size());
    for (size_t i = 0; i < ciphersuites_.size(); ++i) {
        writer.Write(ciphersuites_[i].ciphersuite_code());
    }

    writer.Write(random_.c_str(), random_.size());
    writer.Write(client_gmt_time_);

    int ret = extensions_.Serialize(writer.writer());
    MMTLS_CHECK_EQ(OK, ret, ret, "serialize extensions");

    return OK;
}

int ClientHello::DoDeserialize(DeserializeReader& reader) {
    HandShakeType type = HANDSHAKE_NONE;

    int ret = reader.Get(type);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get type");
    MMTLS_CHECK_EQ(ClientHello::StaticType(),
                   type,
                   ERR_DECODE_ERROR,
                   "type %u vs %u doesn't match",
                   ClientHello::StaticType(),
                   type);

    byte minor_ver = 0, major_ver = 0;
    ret = reader.Get(minor_ver);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get minor version");
    ret = reader.Get(major_ver);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get major version");

    version_ = ProtocolVersion(minor_ver, major_ver);

    byte length = 0;
    ret = reader.Get(length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get ciphersuites size");

    ciphersuites_.clear();
    uint16 ciphersuite_code = 0;
    for (int i = 0; i < length; ++i) {
        ret = reader.Get(ciphersuite_code);
        MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get ciphersute code");
        ciphersuites_.push_back(CipherSuite::GetByCode(ciphersuite_code));
    }

    ret = reader.GetString(random_, FIXED_RANDOM_SIZE);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get client random");

    ret = reader.Get(client_gmt_time_);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get client gmt time");

    ret = extensions_.Init(reader.reader());
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "init extensions");

    return OK;
}

bool ClientHello::operator==(const ClientHello& other) const {
    return ciphersuites_ == other.ciphersuites_ && version_ == other.version_ && random_ == other.random_
           && client_gmt_time_ == other.client_gmt_time_ && extensions_ == other.extensions_;
}

// ServerHello implementation

int ServerHello::DoSerialize(SerializeWriter& writer) {
    writer.Write(GetType());
    writer.Write(version_.MinorVersion());
    writer.Write(version_.MajorVersion());
    writer.Write(ciphersuite_.ciphersuite_code());

    MMTLS_CHECK_EQ(FIXED_RANDOM_SIZE, random_.size(), ERR_ILLEGAL_PARAM, "server random size");
    writer.Write(random_.c_str(), random_.size());

    int ret = extensions_.Serialize(writer.writer());
    MMTLS_CHECK_EQ(OK, ret, ret, "serialize extensions");
    return OK;
}

int ServerHello::DoDeserialize(DeserializeReader& reader) {
    HandShakeType type = HANDSHAKE_NONE;

    int ret = reader.Get(type);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get type");
    MMTLS_CHECK_EQ(ServerHello::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");

    byte minor_ver = 0, major_ver = 0;
    ret = reader.Get(minor_ver);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get minor version");
    ret = reader.Get(major_ver);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get major version");

    version_ = ProtocolVersion(minor_ver, major_ver);

    uint16 ciphersuite_code = 0;
    ret = reader.Get(ciphersuite_code);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get ciphersuite code");

    ciphersuite_ = CipherSuite::GetByCode(ciphersuite_code);

    ret = reader.GetString(random_, FIXED_RANDOM_SIZE);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get server random");

    ret = extensions_.Init(reader.reader());
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "init extensions");

    return ret;
}

bool ServerHello::operator==(const ServerHello& other) const {
    return version_ == other.version_ && ciphersuite_ == other.ciphersuite_ && random_ == other.random_
           && extensions_ == other.extensions_;
}

// NewSessionTicket implementation

NewSessionTicket::~NewSessionTicket() {
    for (size_t i = 0; i < psks_.size(); ++i) {
        if (psks_[i] != NULL) {
            delete psks_[i];
            psks_[i] = NULL;
        }
    }
}

int NewSessionTicket::DoSerialize(SerializeWriter& writer) {
    writer.Write(GetType());

    MMTLS_CHECK_LE(psks_.size(), MAX_NEW_SESSION_TICKET_PSK_SIZE, ERR_ILLEGAL_PARAM, "psk size %zu", psks_.size());
    writer.Write((byte)psks_.size());

    int ret = OK;
    for (size_t i = 0; i < psks_.size(); ++i) {
        MMTLS_CHECK_TRUE(psks_[i] != NULL, ERR_UNEXPECT_CHECK_FAIL, "psk is null");
        ret = psks_[i]->Serialize(writer.writer());
        MMTLS_CHECK_EQ(OK, ret, ret, "serialize psk fail");
    }

    return OK;
}

int NewSessionTicket::DoDeserialize(DeserializeReader& reader) {
    HandShakeType type = HANDSHAKE_NONE;

    int ret = reader.Get(type);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get type fail");
    MMTLS_CHECK_EQ(NewSessionTicket::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");

    byte length = 0;
    ret = reader.Get(length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get length fail");
    MMTLS_CHECK_LE(length, MAX_NEW_SESSION_TICKET_PSK_SIZE, ERR_DECODE_ERROR, "psks size %u invalid", length);

    for (int i = 0; i < length; ++i) {
        Psk* psk = new Psk();
        ret = psk->Init(reader.reader());
        if (OK != ret) {
            delete psk;
            psk = NULL;
        }
        MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "init psk");

        psks_.push_back(psk);
    }

    return OK;
}

bool NewSessionTicket::operator==(const NewSessionTicket& other) const {
    if (psks_.size() != other.psks_.size())
        return false;
    for (size_t i = 0; i < psks_.size(); ++i) {
        if (!(*psks_[i] == *other.psks_[i]))
            return false;
    }
    return true;
}

// EncryptedExtensions implementation

int EncryptedExtensions::DoSerialize(SerializeWriter& writer) {
    writer.Write(GetType());

    int ret = extensions_.Serialize(writer.writer());
    MMTLS_CHECK_EQ(OK, ret, ret, "serialize extensions");

    return OK;
}

int EncryptedExtensions::DoDeserialize(DeserializeReader& reader) {
    HandShakeType type = HANDSHAKE_NONE;

    int ret = reader.Get(type);

    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get type");
    MMTLS_CHECK_EQ(EncryptedExtensions::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");

    ret = extensions_.Init(reader.reader());
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "init extensions");

    return OK;
}

bool EncryptedExtensions::operator==(const EncryptedExtensions& other) const {
    return extensions_ == other.extensions_;
}

// CertificateVerify implementation

int CertificateVerify::DoSerialize(SerializeWriter& writer) {
    writer.Write(GetType());

    MMTLS_CHECK_LE(signature_.size(), MAX_SIGNATURE_SIZE, ERR_ILLEGAL_PARAM, "signature size exceed limit");
    writer.Write((uint16)signature_.size());
    writer.Write(signature_);

    return OK;
}

int CertificateVerify::DoDeserialize(DeserializeReader& reader) {
    HandShakeType type = HANDSHAKE_NONE;

    int ret = reader.Get(type);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get type fail");
    MMTLS_CHECK_EQ(CertificateVerify::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");

    uint16 length = 0;
    ret = reader.Get(length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get signature size fail");

    ret = reader.GetString(signature_, length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get signature fail");

    return OK;
}

bool CertificateVerify::operator==(const CertificateVerify& other) const {
    return signature_ == other.signature_;
}

// Finished implementation

int Finished::DoSerialize(SerializeWriter& writer) {
    writer.Write(GetType());

    MMTLS_CHECK_LE(verify_data_.size(), MAX_FINISHED_VERIFY_DATA_SIZE, ERR_ILLEGAL_PARAM, "verify data size");
    writer.Write((uint16)verify_data_.size());
    writer.Write(verify_data_.c_str(), verify_data_.size());

    return OK;
}

int Finished::DoDeserialize(DeserializeReader& reader) {
    HandShakeType type = HANDSHAKE_NONE;

    int ret = reader.Get(type);

    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get type");
    MMTLS_CHECK_EQ(Finished::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");

    uint16 length = 0;
    ret = reader.Get(length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get verify data size");

    ret = reader.GetString(verify_data_, length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get verify data");

    return OK;
}

bool Finished::operator==(const Finished& other) const {
    return verify_data_ == other.verify_data_;
}

}  // namespace mmtls
