
#ifndef STN_CALLBACK_BRIDGE_H_
#define STN_CALLBACK_BRIDGE_H_

#include <string>
#include "mars/comm/autobuffer.h"
#include "mars/stn/stn.h"

namespace mars {
namespace stn {

class StnCallbackBridge {
 public:
    virtual ~StnCallbackBridge();
    virtual bool MakesureAuthed(const std::string& _host);

    virtual void TrafficData(ssize_t _send, ssize_t _recv);

    virtual std::vector<std::string> OnNewDns(const std::string& host);
    virtual void OnPush(uint64_t _channel_id,
                        uint32_t _cmdid,
                        uint32_t _taskid,
                        const AutoBuffer& _body,
                        const AutoBuffer& _extend);
    virtual bool Req2Buf(uint32_t _taskid,
                         void* const _user_context,
                         AutoBuffer& outbuffer,
                         AutoBuffer& extend,
                         int& error_code,
                         const int channel_select,
                         const std::string& host);
    virtual int Buf2Resp(uint32_t _taskid,
                         void* const _user_context,
                         const AutoBuffer& _inbuffer,
                         const AutoBuffer& _extend,
                         int& _error_code,
                         const int _channel_select);
    virtual int OnTaskEnd(uint32_t _taskid, void* const _user_context, int _error_type, int _error_code);

    virtual void ReportConnectStatus(int _status, int _longlink_status);
    virtual void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port);
    virtual void OnShortLinkNetworkError(ErrCmdType _err_type,
                                         int _err_code,
                                         const std::string& _ip,
                                         const std::string& _host,
                                         uint16_t _port);

    virtual void OnLongLinkStatusChange(int _status);
    //长连信令校验 ECHECK_NOW, ECHECK_NEXT = 1, ECHECK_NEVER = 2 
    virtual int GetLonglinkIdentifyCheckBuffer(AutoBuffer& _identify_buffer, AutoBuffer& _buffer_hash, int32_t& _cmdid);
    //长连信令校验回包 
    virtual bool OnLonglinkIdentifyResponse(const AutoBuffer& _response_buffer,
                                            const AutoBuffer& _identify_buffer_hash);

    virtual void RequestSync();
};

}
}

#endif //STN_CALLBACK_BRIDGE_H_
