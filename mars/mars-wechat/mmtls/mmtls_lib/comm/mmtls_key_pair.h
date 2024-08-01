#pragma once

#include <iostream>
#include <string>

#include "mmtls_constants.h"
#include "mmtls_data_pack.h"
#include "mmtls_data_reader.h"
#include "mmtls_key_pair.h"
#include "mmtls_types.h"

namespace mmtls {

/**
 * mmtls key pair
 */
class KeyPair : public DataPackInterface {
 public:
    KeyPair();

    KeyPair(uint32 version, uint32 nid, const mmtls::String& public_key, const mmtls::String& private_key);
    KeyPair(const KeyPair& other);
    KeyPair& operator=(const KeyPair& other);

    virtual ~KeyPair();

    inline uint32 version() const {
        return version_;
    }
    inline void set_version(uint32 version) {
        version_ = version;
    }

    inline uint32 nid() const {
        return nid_;
    }
    inline void set_nid(uint32 nid) {
        nid_ = nid;
    }

    inline const mmtls::String& public_key() const {
        return public_key_;
    }
    inline void set_public_key(const mmtls::String& public_key) {
        public_key_ = public_key;
    }
    inline void set_public_key(const char* buf, size_t size) {
        public_key_.assign(buf, size);
    }

    inline const mmtls::String& private_key() const {
        return private_key_;
    }
    inline void set_private_key(const mmtls::String& private_key) {
        private_key_ = private_key;
    }
    inline void set_private_key(const char* buf, size_t size) {
        private_key_.assign(buf, size);
    }

    void Clear();

    bool operator==(const KeyPair& other) const;

 protected:
    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    friend std::ostream& operator<<(std::ostream& os, const KeyPair& key);

 protected:
    uint32 version_;
    uint32 nid_;
    mmtls::String public_key_;
    mmtls::String private_key_;
};

}  // namespace mmtls
