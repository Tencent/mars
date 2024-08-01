//#include "iUtils.h"

#include "mmtls_alert.h"

#include "mars/comm/strutil.h"
#include "mmtls_data_reader.h"
#include "mmtls_data_writer.h"

namespace mmtls {

AlertLevel Alert::GetAlertLevel(AlertType type) {
    if (type == ALERT_NONE) {
        return ALERT_LEVEL_NONE;
    } else if (type == ALERT_END_OF_EARLY_DATA) {
        return ALERT_LEVEL_WARNING;
    } else {
        return ALERT_LEVEL_FATAL;
    }
}

bool Alert::IsValid() const {
    return !(type_code_ == ALERT_NONE || level_code_ == ALERT_LEVEL_NONE);
}

void Alert::SetFallBackUrls(const mmtls::String& fallback_urls) {
    fallback_urls_ = fallback_urls;
    fallback_urls_vec_.clear();
    // Comm::StrSplitList(fallback_urls_.c_str(), ";", true, fallback_urls_vec_);
    strutil::SplitToken(std::string(fallback_urls_.c_str()), ";", fallback_urls_vec_);
}

void Alert::SetFallBackUrlsSignature(const mmtls::String& fallback_urls_signature) {
    fallback_urls_signature_ = fallback_urls_signature;
}

bool Alert::HasFallBackUrl(const mmtls::String& url) const {
    return std::find(fallback_urls_vec_.begin(), fallback_urls_vec_.end(), url.std_string())
           != fallback_urls_vec_.end();
}

const mmtls::String& Alert::GetFallBackUrls() const {
    return fallback_urls_;
}

const mmtls::String& Alert::GetFallBackUrlsSignature() const {
    return fallback_urls_signature_;
}

const std::vector<std::string>& Alert::GetFallbackUrlsVec() const {
    return fallback_urls_vec_;
}

int Alert::DoSerialize(SerializeWriter& writer) {
    MMTLS_CHECK_TRUE(IsValid(), ERR_UNEXPECT_CHECK_FAIL, "alert is invalid");

    writer.Write(type_code_);
    writer.Write(level_code_);

    if (type_code_ == ALERT_FALLBACK_NO_MMTLS) {
        // write safe fallback fields
        uint32 length = (uint32)fallback_urls_.size();
        writer.WriteVarintT(length);
        writer.Write(fallback_urls_);

        length = (uint32)fallback_urls_signature_.size();
        writer.WriteVarintT(length);
        writer.Write(fallback_urls_signature_);

        if (HasCertRegion()) {
            writer.Write(cert_region_);
            writer.Write(cert_version_);
        }
    }

    return OK;
}

int Alert::DoDeserialize(DeserializeReader& reader) {
    MMTLS_CHECK_EQ(OK, reader.Get(type_code_), ERR_DECODE_ERROR, "get alert type fail");
    MMTLS_CHECK_EQ(OK, reader.Get(level_code_), ERR_DECODE_ERROR, "get alert level fail");

    if (type_code_ == ALERT_FALLBACK_NO_MMTLS) {
        // read safe fallback fields
        uint32 length = 0;
        mmtls::String tmp_urls;
        MMTLS_CHECK_EQ(OK, reader.GetVarintT(length), ERR_DECODE_ERROR, "get fallback urls size fail");
        MMTLS_CHECK_EQ(OK, reader.GetString(tmp_urls, length), ERR_DECODE_ERROR, "get fallback urls fail");
        SetFallBackUrls(tmp_urls);

        MMTLS_CHECK_EQ(OK, reader.GetVarintT(length), ERR_DECODE_ERROR, "get fallback urls signature size fail");
        MMTLS_CHECK_EQ(OK,
                       reader.GetString(fallback_urls_signature_, length),
                       ERR_DECODE_ERROR,
                       "get fallback urls signature fail");

        MMTLS_CHECK_EQ(OK, reader.Get(cert_region_), ERR_DECODE_ERROR, "get cert region fail");
        MMTLS_CHECK_EQ(OK, reader.Get(cert_version_), ERR_DECODE_ERROR, "get cert version fail");
    }

    MMTLS_CHECK_TRUE(IsValid(), ERR_DECODE_ERROR, "alert is invalid");

    return OK;
}

}  // namespace mmtls
