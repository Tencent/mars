#pragma once

#include <string>
#include <vector>

#include "mmtls_constants.h"
#include "mmtls_data_pack.h"

namespace mmtls {
class DataWriter;
class DataReader;

/**
 * SSL/TLS Alert Message
 */
class Alert : public DataPackInterface {
 public:
    Alert() : level_code_(ALERT_LEVEL_NONE), type_code_(ALERT_NONE), cert_region_(0), cert_version_(0) {
    }
    Alert(AlertLevel level_code, AlertType type_code)
    : level_code_(level_code), type_code_(type_code), cert_region_(0), cert_version_(0) {
    }
    virtual ~Alert() {
    }

    static AlertLevel GetAlertLevel(AlertType type);

    bool IsValid() const;

    AlertType Type() const {
        return type_code_;
    }
    AlertLevel Level() const {
        return level_code_;
    }

    bool HasFallBackUrl(const mmtls::String& url) const;
    void SetFallBackUrls(const mmtls::String& fallback_urls);
    void SetFallBackUrlsSignature(const mmtls::String& fallback_urls_signature);
    const mmtls::String& GetFallBackUrls() const;
    const mmtls::String& GetFallBackUrlsSignature() const;
    const std::vector<std::string>& GetFallbackUrlsVec() const;

    bool HasCertRegion() const {
        return cert_region_ > 0;
    }
    void SetCertRegion(uint32 cert_region) {
        cert_region_ = cert_region;
    }
    const uint32 CertRegion() const {
        return cert_region_;
    }

    void SetCertVersion(uint32 cert_version) {
        cert_version_ = cert_version;
    }
    const uint32 CertVersion() const {
        return cert_version_;
    }

 protected:
    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    AlertLevel level_code_;
    AlertType type_code_;

    // For safe fallback to no mmtls.
    mmtls::String fallback_urls_;
    std::vector<std::string> fallback_urls_vec_;
    mmtls::String fallback_urls_signature_;

    uint32 cert_region_;
    uint32 cert_version_;
};

}  // namespace mmtls
