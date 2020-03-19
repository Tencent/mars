
#include "stn_callback_bridge.h"

namespace mars {
namespace stn {
StnCallbackBridge::~Callback() {

}

bool StnCallbackBridge::MakesureAuthed(const std::string& _host) {
    return false;
}

void StnCallbackBridge::TrafficData(ssize_t _send, ssize_t _recv) {

}

std::vector<std::string> StnCallbackBridge::OnNewDns(const std::string& host) {
    std::vector<std::string> vec;
    return vec;
}

void StnCallbackBridge::OnPush(uint64_t _channel_id,
                               uint32_t _cmdid,
                               uint32_t _taskid,
                               const AutoBuffer& _body,
                               const AutoBuffer& _extend) {

}

bool StnCallbackBridge::Req2Buf(uint32_t _taskid,
                                void* const _user_context,
                                AutoBuffer& outbuffer,
                                AutoBuffer& extend,
                                int& error_code,
                                const int channel_select,
                                const std::string& host) {
    return false;
}

int StnCallbackBridge::Buf2Resp(uint32_t _taskid,
                                void* const _user_context,
                                const AutoBuffer& _inbuffer,
                                const AutoBuffer& _extend,
                                int& _error_code,
                                const int _channel_select) {
    return 0;
}

int StnCallbackBridge::OnTaskEnd(uint32_t _taskid, void* const _user_context, int _error_type, int _error_code) {
    return 0;
}

void StnCallbackBridge::ReportConnectStatus(int _status, int _longlink_status) {

}

void StnCallbackBridge::OnLongLinkNetworkError(ErrCmdType _err_type,
                                               int _err_code,
                                               const std::string& _ip,
                                               uint16_t _port) {

}

void StnCallbackBridge::OnShortLinkNetworkError(ErrCmdType _err_type,
                                                int _err_code,
                                                const std::string& _ip,
                                                const std::string& _host,
                                                uint16_t _port) {

}

void StnCallbackBridge::OnLongLinkStatusChange(int _status) {

}

int StnCallbackBridge::GetLonglinkIdentifyCheckBuffer(AutoBuffer& _identify_buffer,
                                                      AutoBuffer& _buffer_hash,
                                                      int32_t& _cmdid) {
    return 0;
}

bool StnCallbackBridge::OnLonglinkIdentifyResponse(const AutoBuffer& _response_buffer,
                                                   const AutoBuffer& _identify_buffer_hash) {
    return false;
}

void StnCallbackBridge::RequestSync() {

}

}
}