/*
 * shortlink_mmtls_support.h
 *
 *  Created on: Jul 15, 2016
 *      Author: elviswu
 */

#ifndef SRC_MMTLS_SHORTLINK_MMTLS_SUPPORT_H_
#define SRC_MMTLS_SHORTLINK_MMTLS_SUPPORT_H_

#include <functional>

#include "mars/comm/autobuffer.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mmtls_lib/client/mmtls_client_channel.h"
#include "mmtls_lib/comm/mmtls_constants.h"
#include "mmtls_lib/comm/mmtls_event_callback.h"
#include "mmtls_lib/comm/mmtls_string.h"

namespace mmtls {
class ClientChannel;
} /* namespace mmtls */

using mmtls::AlertType;
using mmtls::ClientChannel;
using mmtls::EventCallback;

enum MMTLSState {
    kStateNone,
    kHandshakeDataEncrypt,
    kHandshakeDataSent,
    kHandshakeDataRecv,
    kAppDataEncrypt,
    kAppDataSent,
    kAppDataRecv
};

class ShortLinkEventCallback : public EventCallback {
 public:
    ShortLinkEventCallback()
    : mmtls_state_(kStateNone)
    , is_send_plaintxt_tlv_encoded_(false)
    , is_recv_alert_(false)
    , alert_type_(mmtls::ALERT_NONE) {
    }
    virtual void OnSendTo(mmtls::String* bytes) {
        xassert2(send_ciphertxt_.Length() == 0);
        send_ciphertxt_.Write(bytes->c_str(), bytes->size());
        xdebug2(TSF "%_: OnSendTo called. length:%_", task_tag_.c_str(), send_ciphertxt_.Length());
    }
    virtual void OnReceive(mmtls::String& bytes, const char* appendix = NULL) {
        // xassert2(m_recv_plaintxt.Length() == 0);
        recv_plaintxt_.Write(bytes.c_str(), bytes.size());
        xdebug2(TSF "%_: OnReceive called. bytes.size=%_, m_recv_plaintxt.length=%_",
                task_tag_.c_str(),
                bytes.size(),
                recv_plaintxt_.Length());
    }
    virtual void OnHandShake(const ExportState& state) {
        xdebug2(TSF "%_: OnHandShake called. state=%_, cert_region: %_",
                task_tag_.c_str(),
                state.stage(),
                state.has_cert_region());
        if (state.has_cert_region() && signal_handshake_completed) {
            signal_handshake_completed();
        }
    }
    virtual void OnAlert(AlertType type) {
        is_recv_alert_ = true;
        alert_type_ = type;
        xdebug2(TSF "%_:OnAlert called. alertType=%_", task_tag_.c_str(), type);
    }

    void ResetState() {
        send_ciphertxt_.Reset();
        recv_plaintxt_.Reset();
        mmtls_state_ = kStateNone;
        is_recv_alert_ = false;
        alert_type_ = mmtls::ALERT_NONE;
        // cannot reset m_httpbody_plaintxt and m_isSendPlaintxtTlvEncoded
    }

    AutoBuffer& GetSendCiphertxt() {
        return send_ciphertxt_;
    }
    void ClearSendCiphertxt() {
        send_ciphertxt_.Reset();
    }
    bool IsCiphertxtReady() {
        return (send_ciphertxt_.Length() > 0);
    }

    void SetSendPlaintxt(AutoBuffer& _body) {
        http_body_plaintxt_.Attach(_body);
    }
    AutoBuffer& GetSendPlaintxt() {
        return http_body_plaintxt_;
    }
    void ClearSendPlaintxt() {
        http_body_plaintxt_.Reset();
    }
    bool IsSendPaintxtValid() {
        return http_body_plaintxt_.Length() > 0;
    }

    MMTLSState GetMMTLSState() {
        return mmtls_state_;
    }
    void SetMMTLSState(MMTLSState _state) {
        mmtls_state_ = _state;
    }

    AutoBuffer& GetRecvPlaintxt() {
        return recv_plaintxt_;
    }
    bool IsRevPlaintxtReady() {
        return (recv_plaintxt_.Length() > 0);
    }

    bool IsTlvEncoded() {
        return is_send_plaintxt_tlv_encoded_;
    }
    void SetTlvEncodeStatus(bool _status) {
        is_send_plaintxt_tlv_encoded_ = _status;
    }

    mmtls::AlertType AlertType() {
        return alert_type_;
    }

    mmtls::String& TaskTag() {
        return task_tag_;
    }
    void TaskTag(const mmtls::String& _task_tag) {
        task_tag_ = _task_tag;
    }

 public:
    std::function<void()> signal_handshake_completed;

 private:
    AutoBuffer send_ciphertxt_;
    AutoBuffer http_body_plaintxt_;
    AutoBuffer recv_plaintxt_;

    enum MMTLSState mmtls_state_;

    bool is_send_plaintxt_tlv_encoded_;
    bool is_recv_alert_;

    mmtls::AlertType alert_type_;

    mmtls::String task_tag_;

 private:
    ShortLinkEventCallback(const ShortLinkEventCallback&);
    ShortLinkEventCallback& operator=(const ShortLinkEventCallback&);
};

#endif /* SRC_MMTLS_SHORTLINK_MMTLS_SUPPORT_H_ */
