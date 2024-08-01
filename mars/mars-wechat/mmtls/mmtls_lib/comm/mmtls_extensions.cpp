#include "mmtls_extensions.h"

#include "mmtls_constants.h"
#include "mmtls_types.h"
#include "mmtls_utils.h"

namespace mmtls {

int Extension::DetectExtensionType(const DataReader& reader, ExtensionType& type) {
    return DataPackInterface::DetectPackNumber(type, reader, 0);
}

// EarlyEncryptData implementaion

int EarlyEncryptData::DoSerialize(SerializeWriter& writer) {
    writer.Write(GetType());
    writer.Write(client_gmt_time());
    return OK;
}

int EarlyEncryptData::DoDeserialize(DeserializeReader& reader) {
    ExtensionType type = EXTENSION_NONE;
    int ret = reader.Get(type);

    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get extension type fail");
    MMTLS_CHECK_EQ(EarlyEncryptData::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");
    ret = reader.Get(client_gmt_time_);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get client gmt time fail");

    return OK;
}

bool EarlyEncryptData::operator==(const EarlyEncryptData& other) const {
    return client_gmt_time_ == other.client_gmt_time_;
}

// PreSharedKeyExtension implementation

PreSharedKeyExtension::~PreSharedKeyExtension() {
    if (psk_ != NULL) {
        delete psk_;
        psk_ = NULL;
    }
}

int PreSharedKeyExtension::DoSerialize(SerializeWriter& writer) {
    writer.Write(GetType());

    if (psk_ != NULL) {
        writer.Write((byte)1);
        psk_->Serialize(writer.writer());
    } else {
        writer.Write((byte)0);
    }
    return OK;
}

PreSharedKeyExtension* PreSharedKeyExtension::Clone() const {
    MMTLS_CHECK_TRUE(this->psk_ != NULL, NULL, "psk is null");
    return new PreSharedKeyExtension(new Psk(*(this->psk_)));
}

int PreSharedKeyExtension::DoDeserialize(DeserializeReader& reader) {
    ExtensionType type = EXTENSION_NONE;
    int ret = reader.Get(type);

    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get extension type fail");
    MMTLS_CHECK_EQ(PreSharedKeyExtension::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");

    byte length = 0;
    ret = reader.Get(length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get psk count fail");

    for (int i = 0; i < length; ++i) {
        psk_ = new Psk();
        ret = psk_->Init(reader.reader());
        MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "init psk fail");
    }

    return OK;
}

bool PreSharedKeyExtension::operator==(const PreSharedKeyExtension& other) const {
    MMTLS_CHECK_TRUE(NULL != psk_, false, "psk is null");
    MMTLS_CHECK_TRUE(NULL != other.psk_, false, "psk is null");

    return *psk_ == *other.psk_;
}

// ClientKeyShare implementation

int ClientKeyShare::ClientKeyOffer::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_LE(public_value_.size(),
                   MAX_PUBLIC_VALUE_SIZE,
                   ERR_ILLEGAL_PARAM,
                   "public value size exceed limitation");

    writer.Write(version_);
    writer.Write((uint16)public_value_.size());
    writer.Write(public_value_.c_str(), public_value_.size());

    return OK;
}

int ClientKeyShare::ClientKeyOffer::DoDeserialize(DeserializeReader& reader) {
    int ret = reader.Get(version_);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get key offer version fail");

    uint16 length = 0;
    ret = reader.Get(length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get key offer public value size fail");

    ret = reader.GetString(public_value_, length);

    return ret;
}

bool ClientKeyShare::ClientKeyOffer::operator==(const ClientKeyShare::ClientKeyOffer& other) const {
    return version_ == other.version_ && public_value_ == other.public_value_;
}

int ClientKeyShare::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_LE(key_offers_.size(), MAX_KEY_OFFER_SIZE, ERR_ILLEGAL_PARAM, "key offer size exceed limitation");

    writer.Write(GetType());
    writer.Write((byte)key_offers_.size());

    for (size_t i = 0; i < key_offers_.size(); ++i) {
        key_offers_[i].Serialize(writer.writer());
    }

    // 0用于兼容老版本
    writer.Write((uint32)0);
    writer.Write((byte)certificate_versions_.size());
    for (size_t i = 0; i < certificate_versions_.size(); ++i) {
        writer.Write(certificate_versions_[i]);
    }

    return OK;
}

int ClientKeyShare::DoDeserialize(DeserializeReader& reader) {
    ExtensionType type = EXTENSION_NONE;
    int ret = reader.Get(type);

    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get extension type fail");
    MMTLS_CHECK_EQ(ClientKeyShare::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");

    byte length = 0;
    ret = reader.Get(length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get key offer count fail");

    key_offers_.clear();
    for (int i = 0; i < length; ++i) {
        key_offers_.push_back(ClientKeyOffer());
        ret = key_offers_.rbegin()->Deserialize(reader.reader());
        MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "deserialize key offer fail");
    }

    uint32 probe = 0;
    ret = reader.Get(probe);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get certificate version probe fail");

    if (probe > 0) {
        // 老版本
        certificate_versions_.push_back(probe);
    } else {
        length = 0;
        ret = reader.Get(length);
        MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get certificate count fail");

        certificate_versions_.clear();
        for (int i = 0; i < length; ++i) {
            uint32 certificate_version = 0;
            ret = reader.Get(certificate_version);
            MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get certificate version fail");

            certificate_versions_.push_back(certificate_version);
        }
    }

    return OK;
}

bool ClientKeyShare::operator==(const ClientKeyShare& other) const {
    if (key_offers_.size() != other.key_offers_.size())
        return false;
    if (certificate_versions_.size() != other.certificate_versions_.size())
        return false;
    for (size_t i = 0; i < key_offers_.size(); ++i) {
        if (!(key_offers_[i] == other.key_offers_[i]))
            return false;
    }
    for (size_t i = 0; i < certificate_versions_.size(); ++i) {
        if (!(certificate_versions_[i] == other.certificate_versions_[i]))
            return false;
    }
    return true;
}

// ServerKeyShare implementation

int ServerKeyShare::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_LE(public_value_.size(),
                   MAX_PUBLIC_VALUE_SIZE,
                   ERR_ILLEGAL_PARAM,
                   "public value size exceed limitation");

    writer.Write(GetType());
    writer.Write(version_);
    writer.Write((uint16)public_value_.size());
    writer.Write(public_value_.c_str(), public_value_.size());

    return OK;
}

int ServerKeyShare::DoDeserialize(DeserializeReader& reader) {
    ExtensionType type = EXTENSION_NONE;
    int ret = reader.Get(type);

    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get extension type fail");
    MMTLS_CHECK_EQ(ServerKeyShare::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");

    ret = reader.Get(version_);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get key offer name group fail");

    uint16 length = 0;
    ret = reader.Get(length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get public value size fail");

    ret = reader.GetString(public_value_, length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get public value fail");

    return OK;
}

bool ServerKeyShare::operator==(const ServerKeyShare& other) const {
    return version_ == other.version_ && public_value_ == other.public_value_;
}

// CertRegion implementation

int CertRegion::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_NE(cert_region_, (uint32)0, ERR_ILLEGAL_PARAM, "cert region is 0");
    MMTLS_CHECK_NE(certificate_version_, (uint32)0, ERR_ILLEGAL_PARAM, "certificate version is 0");

    writer.Write(GetType());
    writer.Write(cert_region_);
    writer.Write(certificate_version_);

    return OK;
}

int CertRegion::DoDeserialize(DeserializeReader& reader) {
    ExtensionType type = EXTENSION_NONE;
    int ret = reader.Get(type);

    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get extension type fail");
    MMTLS_CHECK_EQ(CertRegion::StaticType(), type, ERR_DECODE_ERROR, "type doesn't match");

    ret = reader.Get(cert_region_);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get cert region fail");

    ret = reader.Get(certificate_version_);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get certificate version fail");

    return OK;
}

bool CertRegion::operator==(const CertRegion& other) const {
    return (cert_region_ == other.cert_region_) && (certificate_version_ == other.certificate_version_);
}

// Extensions implementation

Extensions::~Extensions() {
    // Delete all extensions.
    for (ExtensionMap::iterator it = extensions_.begin(); it != extensions_.end(); ++it) {
        if (it->second != NULL) {
            delete it->second;
            it->second = NULL;
        }
    }
}

std::set<ExtensionType> Extensions::ExtensionTypes() const {
    std::set<ExtensionType> offers;
    for (ExtensionMap::const_iterator it = extensions_.begin(); it != extensions_.end(); it++) {
        offers.insert(it->first);
    }
    return offers;
}
int Extensions::Init(DataReader& reader) {
    return Deserialize(reader);
}

int Extensions::DoDeserialize(DeserializeReader& reader) {
    byte length = 0;
    int ret = reader.Get(length);
    MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get extensions size fail");

    for (int i = 0; i < length; ++i) {
        ExtensionType type = 0;

        int ret = Extension::DetectExtensionType(reader.reader(), type);
        MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "get extension type fail");

        Extension* ext = NULL;
        switch (type) {
            case EARLY_ENCRYPT_DATA:
                ext = new EarlyEncryptData();
                break;
            case PRE_SHAKE_KEY:
                ext = new PreSharedKeyExtension();
                break;
            case CLIENT_KEY_SHARE:
                ext = new ClientKeyShare();
                break;
            case SERVER_KEY_SHARE:
                ext = new ServerKeyShare();
                break;
            case CERT_REGION:
                ext = new CertRegion();
                break;
            default:
                ext = NULL;
        }

        MMTLS_CHECK_TRUE(ext != NULL, ERR_DECODE_ERROR, "unknown extension type %u", type);

        ret = ext->Deserialize(reader.reader());
        if (OK != ret) {
            if (ext != NULL) {
                delete ext;
                ext = NULL;
            }
            MMTLS_CHECK_EQ(OK, ret, ERR_DECODE_ERROR, "deserialize extension fail");
            return ret;
        }

        extensions_[type] = ext;
    }

    return OK;
}

void Extensions::Add(Extension* ext) {
    if (ext == NULL) {
        MMTLSLOG_ERR("extension is null");
        return;
    }

    ExtensionType type = ext->GetType();
    ExtensionMap::iterator it = extensions_.find(type);
    if (it != extensions_.end()) {
        if (it->second != NULL) {
            delete it->second;
            it->second = NULL;
        }
        extensions_.erase(it);
    }
    extensions_[type] = ext;
}

int Extensions::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_LE(extensions_.size(), MAX_EXTENSION_SIZE, ERR_ILLEGAL_PARAM, "extensions size exceed limitation");

    writer.Write((byte)extensions_.size());

    for (ExtensionMap::const_iterator it = extensions_.begin(); it != extensions_.end(); it++) {
        if (it->second != NULL) {
            int ret = it->second->Serialize(writer.writer());
            MMTLS_CHECK_EQ(OK, ret, ret, "serialize extension fail");
        }
    }

    return OK;
}

bool Extensions::operator==(const Extensions& other) const {
    if (extensions_.size() != other.extensions_.size())
        return false;

    for (ExtensionMap::const_iterator it = extensions_.begin(); it != extensions_.end(); ++it) {
        bool is_equal = false;
        switch (it->first) {
            case EARLY_ENCRYPT_DATA:
                is_equal = *Get<EarlyEncryptData>() == *other.Get<EarlyEncryptData>();
                break;
            case PRE_SHAKE_KEY:
                is_equal = *Get<PreSharedKeyExtension>() == *other.Get<PreSharedKeyExtension>();
                break;
            case CLIENT_KEY_SHARE:
                is_equal = *Get<ClientKeyShare>() == *other.Get<ClientKeyShare>();
                break;
            case SERVER_KEY_SHARE:
                is_equal = *Get<ServerKeyShare>() == *other.Get<ServerKeyShare>();
                break;
            case CERT_REGION:
                is_equal = *Get<CertRegion>() == *other.Get<CertRegion>();
                break;
            default:
                is_equal = false;
                break;
        }
        if (!is_equal)
            return false;
    }

    return true;
}

}  // namespace mmtls
