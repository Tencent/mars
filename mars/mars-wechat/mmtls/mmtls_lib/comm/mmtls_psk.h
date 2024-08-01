#pragma once

#include <string>

#include "mmtls_ciphersuite.h"
#include "mmtls_constants.h"
#include "mmtls_data_pack.h"
#include "mmtls_data_reader.h"
#include "mmtls_data_writer.h"
#include "mmtls_string.h"
#include "mmtls_types.h"
#include "mmtls_version.h"

namespace mmtls {

// The PSK structure.
// See http://tools.ietf.org/html/rfc5077
class Psk : public DataPackInterface {
 public:
    // The ticket part of the PSK.
    class Ticket : public DataPackInterface {
     public:
        Ticket();

        Ticket(PskType type,
               const ProtocolVersion& version,
               const CipherSuite& ciphersuite,
               uint32 key_version,
               uint32 ticket_lifetime_hint,
               const mmtls::String& pre_shared_key,
               const mmtls::String& mac_key,
               uint32 client_gmt_time,
               uint32 server_gmt_time,
               uint32 ecdh_key_version);

        Ticket& operator=(const Ticket& other);

        virtual ~Ticket();

        void Clear();

        PskType type() const {
            return type_;
        }
        const ProtocolVersion& version() const {
            return version_;
        }
        const CipherSuite& ciphersuite() const {
            return ciphersuite_;
        }
        uint32 key_version() const {
            return key_version_;
        }
        uint32 ticket_lifetime_hint() const {
            return ticket_lifetime_hint_;
        }
        const mmtls::String& pre_shared_key() const {
            return pre_shared_key_;
        }
        const mmtls::String& mac_key() const {
            return mac_key_;
        }
        uint32 client_gmt_time() const {
            return client_gmt_time_;
        }
        uint32 server_gmt_time() const {
            return server_gmt_time_;
        }
        uint32 ecdh_key_version() const {
            return ecdh_key_version_;
        }
        bool IsValid() const {
            return is_valid_;
        }

     protected:
        // Serialize object to network order binary data
        virtual int DoSerialize(SerializeWriter& writer);
        // Deserialize from network order binary data
        virtual int DoDeserialize(DeserializeReader& reader);

     private:
        PskType type_;
        ProtocolVersion version_;
        CipherSuite ciphersuite_;

        // The key version for encrypting ticket.
        uint32 key_version_;

        // The seconds that ticket still alive start from server_gmt_time_.
        uint32 ticket_lifetime_hint_;

        // The key material for server.
        mmtls::String pre_shared_key_;

        // The MAC key to verify pre_shared_key_
        mmtls::String mac_key_;

        // The client and server gmt timestamp for anti-replay attack.
        uint32 client_gmt_time_;
        uint32 server_gmt_time_;

        // The ECDH key version of 1-RTT ECDH handshake
        uint32 ecdh_key_version_;

        bool is_valid_;
    };

    // The PSK
    Psk();

    Psk(PskType type,
        uint32 lifetime,
        const mmtls::String& mac_value,
        uint32 key_version,
        const mmtls::String& iv,
        const mmtls::String& encrypted_ticket);

    Psk(const Psk& psk);

    Psk& operator=(const Psk& psk);
    bool operator==(const Psk& psk) const;

    // Clone the psk and clear sensitive fields.
    Psk* SafeClone();

    // Initialize from data reader. This should only be called once.
    int Init(DataReader& reader);

    inline PskType type() const {
        return type_;
    }
    inline uint32 ticket_lifetime_hint() const {
        return ticket_lifetime_hint_;
    }
    inline const mmtls::String& mac_value() const {
        return mac_value_;
    }
    inline uint32 key_version() const {
        return key_version_;
    }
    inline const mmtls::String& iv() const {
        return iv_;
    }
    inline const mmtls::String& encrypted_ticket() const {
        return encrypted_ticket_;
    }

    //需要Init或者设置值的才是valid的
    bool IsValid() const;

 protected:
    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    // The PSK type.
    PskType type_;

    // The seconds that ticket still alive start from that client receive it.
    uint32 ticket_lifetime_hint_;

    // The HMAC value about pre_shared_key
    mmtls::String mac_value_;

    // The version to identify which ticket_key to encrypt PSK ticket
    uint32 key_version_;

    // The IV for AES-GCM
    mmtls::String iv_;

    // The encrypted serialized ticket bytes.
    mmtls::String encrypted_ticket_;
};

}  // namespace mmtls
