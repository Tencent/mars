#pragma once

#include "mmtls_constants.h"
#include "mmtls_string.h"

namespace mmtls {

// The callback interface to handle events happened in mmtls.
class EventCallback {
 public:
    // The state that can be observed by mmtls caller.
    class ExportState {
     public:
        ExportState(HandShakeStage stage) : stage_(stage), cert_region_(0) {
        }

        HandShakeStage stage() const {
            return stage_;
        }

        const bool has_cert_region() const {
            return cert_region_ > 0;
        }
        const uint32 cert_region() const {
            return cert_region_;
        }
        void set_cert_region(uint32 cert_region) {
            cert_region_ = cert_region;
        }

     private:
        HandShakeStage stage_;
        uint32 cert_region_;
    };

    virtual ~EventCallback(){};

    // The callback to let mmtls caller to send the bytes to counterparty.
    virtual void OnSendTo(mmtls::String* bytes) = 0;

    // The callback to let mmtls caller know it receive application data.
    virtual void OnReceive(mmtls::String& bytes, const char* appendix = NULL) = 0;

    // The callback to let mmtls caller know the current handshake state.
    virtual void OnHandShake(const ExportState& state) = 0;

    // The callback to let mmtls caller know errors happened.
    virtual void OnAlert(AlertType type) = 0;
};

}  // namespace mmtls
