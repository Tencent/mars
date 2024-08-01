#pragma once

#include <string>

#include "../../mmtls_client_key.h"
#include "mars/boot/context.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mmtls_channel.h"
#include "mmtls_client_channel_processor.h"
#include "mmtls_client_credential_manager.h"
#include "mmtls_client_handshake_state.h"
#include "mmtls_client_policy.h"
#include "mmtls_client_stat.h"
#include "mmtls_handshake_messages.h"
#include "mmtls_stat.h"
#include "mmtls_string.h"
#include "mmtls_version.h"

namespace mmtls {

class ClientChannel : public Channel {
 public:
    ClientChannel(mars::boot::Context* _context, EventCallback& event_callback, const std::string& _name)
    :  // Delegating constructor is permitted only in c++11 :(
        event_callback_(event_callback)
    , credential_manager_(ClientCredentialManager::GetInstance(_context, _name))
    , policy_(ClientPolicy::GetInstance())
    , send_early_app_data_(false)
    , early_app_data_(NULL)
    , is_long_connection_(false)
    , stat_(&state_)
    , name_(_name) {
    }

    ClientChannel(mars::boot::Context* _context,
                  EventCallback& event_callback,
                  ClientCredentialManager& credential_manager,
                  ClientPolicy& policy,
                  const std::string& _name)
    : event_callback_(event_callback)
    , credential_manager_(credential_manager)
    , policy_(policy)
    , send_early_app_data_(false)
    , early_app_data_(NULL)
    , is_long_connection_(false)
    , stat_(&state_)
    , name_(_name) {
    }

    virtual ~ClientChannel();

    // Initialize the client channel.
    // If app_data is not null, it will try to send the application data during handshake (0-RTT handshake).
    //
    // The above parameters solve the requirements for mmproxy and mmwebproxy
    // For mmwebproxy:
    //   app_data points to application data
    //   is_long_connection is false -- for short connection
    // For mmproxy:
    //   app_data is null
    //   is_long_connection is true -- for long connection
    virtual int Init(const mmtls::String* app_data, bool is_long_connection);

    virtual int Send(const mmtls::String& bytes);

    virtual int SendHeartbeat(const mmtls::String& payload);

    virtual int Receive(const char* bytes, size_t size, size_t& size_hint);

    void SetServerUrl(const char* server_url) {
        server_url_ = server_url;
    }

    const ClientStat& Stat() const {
        return stat_;
    }

    const bool HasCertRegion() const {
        return state_.has_cert_region();
    }
    const uint32 GetCertRegion() const {
        return state_.cert_region();
    }
    const std::vector<std::string>& GetFallbackUrlsVec() const {
        return state_.fallback_urls_vec();
    }

 protected:
    virtual int DoHandShakeLoop(mmtls::String* plain_text, HandShakeMessage* msg);

    virtual int DoHandleError(AlertType alert_type, int error_code);

    virtual int DoReceiveRecord(const RecordHead& record, mmtls::String& plain_text, const byte* payload);

    virtual int DoReceiveAlert(const Alert& alert);

    int WriteMsgToSendBuffer(HandShakeType type, HandShakeMessage* msg);

    bool IsRecvMsgExpected(HandShakeMessage* msg);

    int DoInit();

    int DoSendClientHello();

    int DoSendEncryptedExtension();

    int DoSendEarlyAppData();

    int DoReceiveServerHello(const mmtls::String& plain_text, HandShakeMessage* msg);

    int DoReceiveCertificateVerify(const mmtls::String& plain_text, HandShakeMessage* msg);

    int DoReceiveNewSessionTicket(const mmtls::String& plain_text, HandShakeMessage* msg);

    int DoReceiveServerFinished(const mmtls::String& plain_text, HandShakeMessage* msg);

    int DoSavePsks();

    int DoReceiveEarlyAppData();

    int DoSendClientFinished();

    int DoComplete();

 public:
    // public for testing.
    ClientHandShakeState& client_handshake_state() {
        return state_;
    }

 protected:
    // Override methods

    virtual EventCallback& event_callback() {
        return event_callback_;
    }
    virtual HandShakeState& state() {
        return state_;
    }
    virtual CredentialManager& credential_manager() {
        return credential_manager_;
    }
    virtual Policy& policy() {
        return policy_;
    }
    virtual void FinalReport();
    virtual mmtls::Stat& inner_stat() {
        return stat_;
    }

    int AuditForReceiveEarlyData();
    int AuditFor1stSendToPeer();
    int AduitForActiveSession();

 private:
    EventCallback& event_callback_;
    ClientCredentialManager& credential_manager_;
    ClientPolicy& policy_;
    ClientChannelProcessor processor_;
    ClientHandShakeState state_;

    bool send_early_app_data_;
    const mmtls::String* early_app_data_;
    bool is_long_connection_;
    mmtls::String server_url_;

    ClientStat stat_;
    std::string name_;
};

}  // namespace mmtls
