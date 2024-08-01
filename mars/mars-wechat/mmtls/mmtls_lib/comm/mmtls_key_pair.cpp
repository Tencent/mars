#include "mmtls_key_pair.h"

namespace mmtls {

KeyPair::KeyPair() : version_(0), nid_(0), public_key_(""), private_key_("") {
}

KeyPair::KeyPair(uint32 version, uint32 nid, const mmtls::String& public_key, const mmtls::String& private_key)
: version_(version), nid_(nid), public_key_(public_key), private_key_(private_key) {
}

KeyPair::KeyPair(const KeyPair& other) {
    this->operator=(other);
}
KeyPair& KeyPair::operator=(const KeyPair& other) {
    version_ = other.version();
    nid_ = other.nid();
    public_key_ = other.public_key();
    private_key_ = other.private_key();
    return *this;
}

KeyPair::~KeyPair() {
    public_key_.safe_clear();
    private_key_.safe_clear();
}

// Serialize object to network order binary data
int KeyPair::DoSerialize(SerializeWriter& writer) {
    bool ret = false;

    // set version
    writer.Write(version_);

    // set nid
    writer.Write(nid_);

    // set public key
    ret = public_key_.size() < (uint16)-1;
    MMTLS_CHECK_TRUE(ret, ERR_ILLEGAL_PARAM, "public key is too long");

    writer.Write((uint16)public_key_.size());
    writer.Write(public_key_);

    // set private key
    ret = private_key_.size() < (uint16)-1;
    MMTLS_CHECK_TRUE(ret, ERR_ILLEGAL_PARAM, "private key is too long");

    writer.Write((uint16)private_key_.size());
    writer.Write(private_key_);

    return OK;
}

// Deserialize from network order binary data
int KeyPair::DoDeserialize(DeserializeReader& reader) {
    int ret = 0;
    uint16 len = 0;

    // get version
    ret = reader.Get(version_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get version failed");

    // get nid
    ret = reader.Get(nid_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get nid failed");

    // get public key
    ret = reader.Get(len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get public key len failed");
    ret = reader.GetString(public_key_, len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get public key failed");

    // get private key
    ret = reader.Get(len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get private key len failed");
    ret = reader.GetString(private_key_, len);
    MMTLS_CHECK_EQ(ret, OK, ret, "get private key failed");

    return OK;
}

bool KeyPair::operator==(const KeyPair& other) const {
    return (version_ == other.version() && nid_ == other.nid() && public_key_ == other.public_key()
            && private_key_ == other.private_key());
}

void KeyPair::Clear() {
    version_ = 0;
    nid_ = 0;
    public_key_.safe_clear();
    private_key_.safe_clear();
    public_key_.clear();
    private_key_.clear();
}
std::ostream& operator<<(std::ostream& os, const KeyPair& key) {
    os << "[" << key.version_ << "," << key.nid_ << "," << mmtls::StrToHex(key.public_key_) << ","
       << mmtls::StrToHex(key.private_key_) << "]";
    return os;
}

}  // namespace mmtls
