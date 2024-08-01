#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "mmtls_constants.h"
#include "mmtls_data_pack.h"
#include "mmtls_data_reader.h"
#include "mmtls_data_writer.h"
#include "mmtls_psk.h"
#include "mmtls_string.h"
#include "mmtls_types.h"

namespace mmtls {

// The handshake message extension interface.
class Extension : public DataPackInterface {
 public:
    static int DetectExtensionType(const DataReader& reader, ExtensionType& type);
    friend class Extensions;

    virtual ~Extension() {
    }

    virtual ExtensionType GetType() const = 0;

    // Server can send empty extension to indicate it accept an extension.
    virtual bool Empty() const = 0;
};

class EarlyEncryptData : public Extension {
 public:
    friend class Extensions;

    static ExtensionType StaticType() {
        return EARLY_ENCRYPT_DATA;
    }

    EarlyEncryptData() : client_gmt_time_(0) {
    }

    EarlyEncryptData(uint32 client_gmt_time) : client_gmt_time_(client_gmt_time) {
    }

    virtual ~EarlyEncryptData() {
    }

    virtual ExtensionType GetType() const {
        return StaticType();
    }

    // EarlyEncryptData won't be empty.
    virtual bool Empty() const {
        return false;
    }

    uint32 client_gmt_time() const {
        return client_gmt_time_;
    }

 protected:
    bool operator==(const EarlyEncryptData& other) const;

    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

    uint32 client_gmt_time_;
};

class PreSharedKeyExtension : public Extension {
 public:
    friend class Extensions;

    static ExtensionType StaticType() {
        return PRE_SHAKE_KEY;
    }

    PreSharedKeyExtension() : psk_(NULL) {
    }

    PreSharedKeyExtension(Psk* psk) : psk_(psk) {
    }
    PreSharedKeyExtension* Clone() const;

    virtual ~PreSharedKeyExtension();

    virtual ExtensionType GetType() const {
        return StaticType();
    }

    virtual bool Empty() const {
        return psk_ == NULL;
    }

    Psk* psk() {
        return psk_;
    }

 protected:
    bool operator==(const PreSharedKeyExtension& other) const;

    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    Psk* psk_;
};

class ClientKeyShare : public Extension {
 public:
    friend class Extensions;

    class ClientKeyOffer : public DataPackInterface {
     public:
        friend class ClientKeyShare;

        ClientKeyOffer() : version_(0), public_value_("") {
        }

        ClientKeyOffer(uint32 version, const mmtls::String& public_value)
        : version_(version), public_value_(public_value) {
        }

        uint32 version() const {
            return version_;
        }
        const mmtls::String& public_value() const {
            return public_value_;
        }

     protected:
        bool operator==(const ClientKeyOffer& other) const;

        // Serialize object to network order binary data
        virtual int DoSerialize(SerializeWriter& writer);
        // Deserialize from network order binary data
        virtual int DoDeserialize(DeserializeReader& reader);

     private:
        uint32 version_;
        mmtls::String public_value_;
    };

    static ExtensionType StaticType() {
        return CLIENT_KEY_SHARE;
    }

    // ClientKeyShare() : certificate_version_(0) {}
    ClientKeyShare() {
    }

    // ClientKeyShare(const std::vector<ClientKeyOffer>& key_offers, uint32 certificate_version) :
    // key_offers_(key_offers) ,certificate_version_(certificate_version){}
    ClientKeyShare(const std::vector<ClientKeyOffer>& key_offers, const std::vector<uint32>& certificate_versions)
    : key_offers_(key_offers), certificate_versions_(certificate_versions) {
    }

    virtual ~ClientKeyShare() {
    }

    virtual ExtensionType GetType() const {
        return StaticType();
    }

    // ClientKeyShare won't be empty.
    virtual bool Empty() const {
        return false;
    }

    const std::vector<ClientKeyOffer>& key_offers() {
        return key_offers_;
    }
    // uint32 certificate_version() const { return certificate_version_;}
    const std::vector<uint32>& certificate_versions() {
        return certificate_versions_;
    }

 protected:
    bool operator==(const ClientKeyShare& other) const;

    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    std::vector<ClientKeyOffer> key_offers_;
    // uint32 certificate_version_;
    std::vector<uint32> certificate_versions_;
};

class ServerKeyShare : public Extension {
 public:
    friend class Extensions;

    static ExtensionType StaticType() {
        return SERVER_KEY_SHARE;
    }

    ServerKeyShare() : version_(0), public_value_("") {
    }

    ServerKeyShare(uint32 version, const mmtls::String& public_value) : version_(version), public_value_(public_value) {
    }

    virtual ~ServerKeyShare() {
    }

    virtual ExtensionType GetType() const {
        return StaticType();
    }

    // ServerKeyShare won't be empty
    virtual bool Empty() const {
        return false;
    }

    uint32 version() const {
        return version_;
    }

    const mmtls::String& public_value() const {
        return public_value_;
    }

 protected:
    bool operator==(const ServerKeyShare& other) const;

    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    uint32 version_;

    mmtls::String public_value_;
};

class CertRegion : public Extension {
 public:
    friend class Extensions;

    static ExtensionType StaticType() {
        return CERT_REGION;
    }

    CertRegion() : cert_region_(0), certificate_version_(0) {
    }

    CertRegion(uint32 cert_region, uint32 certificate_version)
    : cert_region_(cert_region), certificate_version_(certificate_version) {
    }

    virtual ~CertRegion() {
    }

    virtual ExtensionType GetType() const {
        return StaticType();
    }

    // CertRegion won't be empty.
    virtual bool Empty() const {
        return false;
    }

    uint32 cert_region() const {
        return cert_region_;
    }
    uint32 certificate_version() const {
        return certificate_version_;
    }

 protected:
    bool operator==(const CertRegion& other) const;

    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    uint32 cert_region_;
    uint32 certificate_version_;
};

class Extensions : public DataPackInterface {
 public:
    typedef std::map<ExtensionType, Extension*> ExtensionMap;

    Extensions() {
    }

    ~Extensions();

    std::set<ExtensionType> ExtensionTypes() const;

    template <typename T>
    T* Get() const {
        ExtensionType type = T::StaticType();
        ExtensionMap::const_iterator it = extensions_.find(type);
        if (it != extensions_.end()) {
            return dynamic_cast<T*>(it->second);
        }
        return NULL;
    }

    template <typename T>
    bool Has() const {
        return (Get<T>() != NULL);
    }

    // Initialize from data reader. This should only be called once.
    int Init(DataReader& reader);

    // Add an extension. The ext will be deleted if Extensions is disposed.
    void Add(Extension* ext);

    bool operator==(const Extensions& other) const;

 protected:
    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    Extensions(const Extensions& other) {
    }
    Extensions& operator=(const Extensions&) {
        return (*this);
    }

 private:
    ExtensionMap extensions_;
};

}  // namespace mmtls
