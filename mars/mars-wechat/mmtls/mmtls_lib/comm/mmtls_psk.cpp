#include "mmtls_psk.h"

namespace mmtls {

/**
 * The plaintext ticket
 */
Psk::Ticket::Ticket()
: type_(PSK_NONE)
, version_(ProtocolVersion::TLS_VNONE)
, ciphersuite_()
, key_version_(0)
, ticket_lifetime_hint_(0)
, pre_shared_key_("")
, mac_key_("")
, client_gmt_time_(0)
, server_gmt_time_(0)
, ecdh_key_version_(0)
, is_valid_(false) {
}

Psk::Ticket::Ticket(PskType type,
                    const ProtocolVersion& version,
                    const CipherSuite& ciphersuite,
                    uint32 key_version,
                    uint32 ticket_lifetime_hint,
                    const mmtls::String& pre_shared_key,
                    const mmtls::String& mac_key,
                    uint32 client_gmt_time,
                    uint32 server_gmt_time,
                    uint32 ecdh_key_version)
: type_(type)
, version_(version)
, ciphersuite_(ciphersuite)
, key_version_(key_version)
, ticket_lifetime_hint_(ticket_lifetime_hint)
, pre_shared_key_(pre_shared_key)
, mac_key_(mac_key)
, client_gmt_time_(client_gmt_time)
, server_gmt_time_(server_gmt_time)
, ecdh_key_version_(ecdh_key_version)
, is_valid_(true)

{
}

Psk::Ticket& Psk::Ticket::operator=(const Psk::Ticket& other) {
    type_ = other.type();
    version_ = other.version();
    ciphersuite_ = other.ciphersuite();
    key_version_ = other.key_version();
    ticket_lifetime_hint_ = other.ticket_lifetime_hint();
    pre_shared_key_ = other.pre_shared_key();
    mac_key_ = other.mac_key();
    client_gmt_time_ = other.client_gmt_time();
    server_gmt_time_ = other.server_gmt_time();
    ecdh_key_version_ = other.ecdh_key_version();
    is_valid_ = other.IsValid();
    return *this;
}

Psk::Ticket::~Ticket() {
}

void Psk::Ticket::Clear() {
    pre_shared_key_.clear();
    mac_key_.clear();
}

// Serialize object to network order binary data
int Psk::Ticket::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_TRUE(IsValid(), ERR_UNEXPECT_CHECK_FAIL, "psk ticket is valid");

    // Below are ticket version 1 wire format serialization.

    writer.Write((byte)(PSK_TICKET_VERSION_1 << 4 | type_));
    writer.Write(version_.version());
    writer.Write(ciphersuite_.ciphersuite_code());
    writer.WriteVarintT(key_version_);
    writer.WriteVarintT(ticket_lifetime_hint_);
    writer.WriteVarintT((uint16)pre_shared_key_.size());
    writer.Write(pre_shared_key_);
    writer.WriteVarintT((uint16)mac_key_.size());
    writer.Write(mac_key_);
    writer.Write(client_gmt_time_);
    writer.Write(server_gmt_time_);
    writer.WriteVarintT(ecdh_key_version_);

    return OK;

    // Below are ticket version 0 wire format serialization. Only some old client will use this format.
    // Server will reject version 0 ticket.

    // // set type
    // writer.Write(type_);

    // // set version
    // DataWriter version_writer;
    // ret = version_.Serialize(version_writer);
    // MMTLS_CHECK_EQ(ret, OK, ret, "serialize version fialed");
    // writer.Write((uint16)version_writer.Size());
    // writer.Write(version_writer.Buffer());

    // // set ciphersuite
    // DataWriter cs_writer;
    // ret =  ciphersuite_.Serialize(cs_writer);
    // MMTLS_CHECK_EQ(ret, OK, ret, "serialize ciphersuite failed");
    // writer.Write((uint16)cs_writer.Size());
    // writer.Write(cs_writer.Buffer());

    // // set key_version
    // writer.Write(key_version_);

    // // set ticket_lifetime_hint
    // writer.Write(ticket_lifetime_hint_);

    // // set pre_shared_key
    // writer.Write((uint16)pre_shared_key_.size());
    // writer.Write(pre_shared_key_);

    // // set mac_key
    // writer.Write((uint16)mac_key_.size());
    // writer.Write(mac_key_);

    // // set client_identity
    // writer.Write((uint16)client_identity_.size());
    // writer.Write(client_identity_);

    // // set client_gmt_time
    // writer.Write(client_gmt_time_);

    // // set server_gmt_time
    // writer.Write(server_gmt_time_);

    // // set ecdh_key_version
    // writer.Write(ecdh_key_version_);
}

// Deserialize from network order binary data
int Psk::Ticket::DoDeserialize(DeserializeReader& reader) {
    int ret = 0;

    ret = reader.Get(type_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get psk type fail");
    MMTLS_CHECK_TRUE((type_ >> 4) == PSK_TICKET_VERSION_1,
                     ERR_UNSUPPORTED_PSK_TICKET_VERSION,
                     "psk ticket version not supported %d",
                     type_ >> 4);
    type_ &= 0x0F;

    uint16 version_code = 0;
    ret = reader.Get(version_code);
    MMTLS_CHECK_EQ(ret, OK, ret, "get mmtls version fail");
    version_ = ProtocolVersion(version_code);
    MMTLS_CHECK_TRUE(version_.IsValid(), ERR_DECODE_ERROR, "mmtls version invalid, version_code %x", version_code);

    uint16 ciphersuite_code = 0;
    ret = reader.Get(ciphersuite_code);
    MMTLS_CHECK_EQ(ret, OK, ret, "get mmtls ciphersuite fail");
    ciphersuite_ = CipherSuite::GetByCode(ciphersuite_code);
    MMTLS_CHECK_TRUE(ciphersuite_.IsValid(),
                     ERR_DECODE_ERROR,
                     "mmtls ciphersuite code invalid, ciphersuite_code %d",
                     ciphersuite_code);

    ret = reader.GetVarintT(key_version_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get key version fail");

    ret = reader.GetVarintT(ticket_lifetime_hint_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get ticket lifetime hint fail");

    uint16 length = 0;
    ret = reader.GetVarintT(length);
    MMTLS_CHECK_EQ(ret, OK, ret, "get pre shared key length fail");
    ret = reader.GetString(pre_shared_key_, length);
    MMTLS_CHECK_EQ(ret, OK, ret, "get pre shared key fail");

    ret = reader.GetVarintT(length);
    MMTLS_CHECK_EQ(ret, OK, ret, "get mac key length fail");
    ret = reader.GetString(mac_key_, length);
    MMTLS_CHECK_EQ(ret, OK, ret, "get mac key fail");

    ret = reader.Get(client_gmt_time_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get client gmt time fail");

    ret = reader.Get(server_gmt_time_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get sever gmt time fail");

    ret = reader.GetVarintT(ecdh_key_version_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get ecdh key version fail");

    is_valid_ = true;
    return OK;

    // Below are ticket version 0 wire format deserialization. Only some old client will use this format.
    // Server will reject version 0 ticket.
    //
    // // get type
    // ret = reader.Get(type_);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get type fialed");

    // // get version
    // mmtls::String version_buffer;
    // ret = reader.Get(len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get version len failed");
    // ret = reader.GetString(version_buffer, len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get version buffer failed");

    // DataReader version_reader(version_buffer);
    // ret = version_.Deserialize(version_reader);
    // MMTLS_CHECK_EQ(ret, OK, ret, "deserialize version fialed");

    // // get ciphersuite
    // mmtls::String cs_buffer;
    // ret = reader.Get(len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get ciphersuite len failed");
    // ret = reader.GetString(cs_buffer, len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get ciphersuite buffer failed");

    // DataReader cs_reader(cs_buffer);
    // ret = ciphersuite_.Deserialize(cs_reader);
    // MMTLS_CHECK_EQ(ret, OK, ret, "deserialize ciphersutie failed");

    // // get key_version
    // ret = reader.Get(key_version_);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get key_version failed");

    // // get ticket_lifetime_hint
    // ret = reader.Get(ticket_lifetime_hint_);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get ticket_lifetime_hint failed");

    // // get pre_shared_key
    // ret = reader.Get(len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get pre_shared_key len failed");
    // ret = reader.GetString(pre_shared_key_, len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get pre_shared_key failed");

    // // get mac_key
    // ret = reader.Get(len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get mac_key len failed");
    // ret = reader.GetString(mac_key_, len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get mac_key, failed");

    // // get client_identity
    // ret = reader.Get(len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get client_identity len failed");
    // ret = reader.GetString(client_identity_, len);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get client_identity failed");

    // // get client_gmt_time
    // ret = reader.Get(client_gmt_time_);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get client_gmt_time failed");

    // // get server_gmt_time
    // ret = reader.Get(server_gmt_time_);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get server_gmt_time failed");

    // // get ecdh_key_version
    // ret = reader.Get(ecdh_key_version_);
    // MMTLS_CHECK_EQ(ret, OK, ret, "get ecdh_key_version failed");
}

/**
 * The PSK
 */
Psk::Psk()
: type_(PSK_NONE), ticket_lifetime_hint_(0), mac_value_(""), key_version_(0), iv_(""), encrypted_ticket_("") {
}

Psk::Psk(PskType type,
         uint32 lifetime,
         const mmtls::String& mac_value,
         uint32 key_version,
         const mmtls::String& iv,
         const mmtls::String& encrypted_ticket)
: type_(type)
, ticket_lifetime_hint_(lifetime)
, mac_value_(mac_value)
, key_version_(key_version)
, iv_(iv)
, encrypted_ticket_(encrypted_ticket) {
}

Psk::Psk(const Psk& psk) {
    type_ = psk.type();
    ticket_lifetime_hint_ = psk.ticket_lifetime_hint();
    mac_value_ = psk.mac_value();
    key_version_ = psk.key_version();
    iv_ = psk.iv();
    encrypted_ticket_ = psk.encrypted_ticket();
}

Psk& Psk::operator=(const Psk& psk) {
    type_ = psk.type();
    ticket_lifetime_hint_ = psk.ticket_lifetime_hint();
    mac_value_ = psk.mac_value();
    key_version_ = psk.key_version();
    iv_ = psk.iv();
    encrypted_ticket_ = psk.encrypted_ticket();

    return *this;
}

bool Psk::operator==(const Psk& psk) const {
    return psk.type_ == type_ && psk.ticket_lifetime_hint_ == ticket_lifetime_hint_ && psk.mac_value_ == mac_value_
           && psk.key_version_ == key_version_ && psk.iv_ == iv_ && psk.encrypted_ticket_ == encrypted_ticket_;
}

Psk* Psk::SafeClone() {
    Psk* psk = new Psk(*this);
    psk->mac_value_.clear();

    return psk;
}

int Psk::Init(DataReader& reader) {
    return Deserialize(reader);
}
int Psk::DoDeserialize(DeserializeReader& reader) {
    uint16 len = 0;
    // get type
    int ret = reader.Get(type_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get type failed");

    // get ticket_lifetime_hint
    ret = reader.Get(ticket_lifetime_hint_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get ticket_lifetime_hint failed");

    // get mac_value
    ret = reader.Get(len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get mac_value len failed");
    ret = reader.GetString(mac_value_, len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get mac_value failed");

    // get key_version
    ret = reader.Get(key_version_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get key_version failed");

    // get iv
    ret = reader.Get(len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get iv len failed");
    ret = reader.GetString(iv_, len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get iv failed");

    // get encrypted_ticket
    ret = reader.Get(len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get encrypted_ticket len failed");

    ret = reader.GetString(encrypted_ticket_, len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get encrypted_ticket failed");

    return OK;
}

int Psk::DoSerialize(SerializeWriter& writer) {
    // set type
    writer.Write(type_);

    // set ticket lifetime hint
    writer.Write(ticket_lifetime_hint_);

    // set mac value
    writer.Write((uint16)mac_value_.size());
    writer.Write(mac_value_);

    // set key version
    writer.Write(key_version_);

    // set iv
    writer.Write((uint16)iv_.size());
    writer.Write(iv_);

    // set encrypted ticket
    writer.Write((uint16)encrypted_ticket_.size());
    writer.Write(encrypted_ticket_);

    return OK;
}

}  // namespace mmtls
