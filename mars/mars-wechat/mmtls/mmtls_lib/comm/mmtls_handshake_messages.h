#pragma once

#include <string>
#include <vector>

#include "mmtls_constants.h"
#include "mmtls_data_pack.h"
#include "mmtls_data_reader.h"
#include "mmtls_data_writer.h"
#include "mmtls_extensions.h"
#include "mmtls_psk.h"
#include "mmtls_string.h"
#include "mmtls_version.h"

namespace mmtls {

// The handshake message interface.
class HandShakeMessage : public DataPackInterface {
 public:
    static int DetectHandShakeType(const DataReader& reader, HandShakeType& type);
    friend class HandShakeState;

    virtual ~HandShakeMessage() {
    }

    virtual HandShakeType GetType() const = 0;

    // Get Extensions object to manager extensions.
    Extensions* extensions() {
        return &extensions_;
    }

 protected:
    Extensions extensions_;
};

class ClientHello : public HandShakeMessage {
 public:
    friend class HandShakeState;

    static HandShakeType StaticType() {
        return CLIENT_HELLO;
    }

    ClientHello() : version_(ProtocolVersion(ProtocolVersion::TLS_VNONE)), random_(""), client_gmt_time_(0) {
    }

    ClientHello(const ProtocolVersion& version,
                const std::vector<CipherSuite>& ciphersuites,
                const mmtls::String& random,
                uint32 client_gmt_time)
    : version_(version), ciphersuites_(ciphersuites), random_(random), client_gmt_time_(client_gmt_time) {
    }

    virtual ~ClientHello(){};

    virtual HandShakeType GetType() const {
        return StaticType();
    }

    const ProtocolVersion& version() const {
        return version_;
    }

    const std::vector<CipherSuite>& ciphersuites() const {
        return ciphersuites_;
    }

    const mmtls::String& random() const {
        return random_;
    }

    uint32 client_gmt_time() const {
        return client_gmt_time_;
    }

 protected:
    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

    bool operator==(const ClientHello& other) const;

 private:
    ProtocolVersion version_;
    std::vector<CipherSuite> ciphersuites_;
    mmtls::String random_;
    uint32 client_gmt_time_;
};

class ServerHello : public HandShakeMessage {
 public:
    friend class HandShakeState;

    static HandShakeType StaticType() {
        return SERVER_HELLO;
    }

    ServerHello()
    : version_(ProtocolVersion(ProtocolVersion::TLS_VNONE)), ciphersuite_(CipherSuite::GetByCode(0)), random_("") {
    }

    ServerHello(const ProtocolVersion& version, const CipherSuite& ciphersuite, const mmtls::String& random)
    : version_(version), ciphersuite_(ciphersuite), random_(random) {
    }

    virtual ~ServerHello(){};

    virtual HandShakeType GetType() const {
        return StaticType();
    }

    const ProtocolVersion& version() const {
        return version_;
    }

    const CipherSuite& ciphersuite() const {
        return ciphersuite_;
    }

    const mmtls::String& random() const {
        return random_;
    }

 protected:
    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

    bool operator==(const ServerHello& other) const;

 private:
    ProtocolVersion version_;
    CipherSuite ciphersuite_;
    mmtls::String random_;
};

class NewSessionTicket : public HandShakeMessage {
 public:
    friend class HandShakeState;

    static HandShakeType StaticType() {
        return NEW_SESSION_TICKET;
    }

    NewSessionTicket() {
    }

    NewSessionTicket(const std::vector<Psk*>& psks) : psks_(psks) {
    }

    virtual ~NewSessionTicket();

    virtual HandShakeType GetType() const {
        return StaticType();
    }

    const std::vector<Psk*>& psks() const {
        return psks_;
    }

 protected:
    Extensions* extensions() {
        return &extensions_;
    }

    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

    bool operator==(const NewSessionTicket& other) const;

 private:
    std::vector<Psk*> psks_;
};

class EncryptedExtensions : public HandShakeMessage {
 public:
    friend class HandShakeState;

    static HandShakeType StaticType() {
        return ENCRYPTED_EXTENSIONS;
    }

    EncryptedExtensions() {
    }

    virtual ~EncryptedExtensions(){};

    virtual HandShakeType GetType() const {
        return StaticType();
    }

 protected:
    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

    bool operator==(const EncryptedExtensions& other) const;
};

class CertificateVerify : public HandShakeMessage {
 public:
    friend class HandShakeState;

    static HandShakeType StaticType() {
        return CERTIFICATE_VERIFY;
    }

    CertificateVerify() {
    }

    CertificateVerify(const mmtls::String& signature) : signature_(signature) {
    }

    virtual ~CertificateVerify(){};

    virtual HandShakeType GetType() const {
        return StaticType();
    }

    const mmtls::String& signature() const {
        return signature_;
    }

 protected:
    bool operator==(const CertificateVerify& other) const;

    Extensions* extensions() {
        return &extensions_;
    }

    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    mmtls::String signature_;
};

class Finished : public HandShakeMessage {
 public:
    friend class HandShakeState;

    static HandShakeType StaticType() {
        return FINISHED;
    }

    Finished() {
    }

    Finished(const mmtls::String& verify_data) : verify_data_(verify_data) {
    }

    virtual ~Finished(){};

    virtual HandShakeType GetType() const {
        return StaticType();
    }

    const mmtls::String& verify_data() const {
        return verify_data_;
    }

    bool operator==(const Finished& other) const;

 protected:
    Extensions* extensions() {
        return &extensions_;
    }

    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    mmtls::String verify_data_;
};

}  // namespace mmtls
