#include "shortlink_empty_impl.h"

#include <string>
#include "mars/comm/autobuffer.h"
#include "mars/comm/move_wrapper.h"
#include "mars/stn/task_profile.h"

#define NO_FIND_CALLBACK_ERROR(id, cmdid) xerror2(TSF"no find callback bridge: %_, %_", id, cmdid)

namespace mars {

namespace stn {

ShortlinkEmptyImpl::ShortlinkEmptyImpl(const Task& _task, std::shared_ptr<ShortlinkCallbackBridge> _callback_bridge)
: task_(_task)
, runner_(std::bind(&ShortlinkEmptyImpl::_StartInternal, this))
, shortlink_callback_bridge_(_callback_bridge) {
    wrapper_.SetShortLinkInterface(this);
    auto cb = shortlink_callback_bridge_.lock();
    if (cb) {
        cb->SetShortlinkWrapper(&wrapper_);
    } else {
        NO_FIND_CALLBACK_ERROR(task_.taskid, task_.cmdid);
    }
}

ShortlinkEmptyImpl::~ShortlinkEmptyImpl() {
    auto cb = shortlink_callback_bridge_.lock();
    if (cb) {
        cb->CancelTask();
        runner_.Join();
    } else {
        NO_FIND_CALLBACK_ERROR(task_.taskid, task_.cmdid);
        runner_.Detach();
        // detach之后可能会crash，因为上层没有收到cancel的回调，可能还会继续调用
        // ShortLinkInterface的函数，但是此时ShortLinkInterface已经析构，导致上层调用空指针
    }
}

void ShortlinkEmptyImpl::SendRequest(AutoBuffer& _buffer_req, AutoBuffer& _buffer_extend) {
    xverbose_function();
    xdebug2(XTHIS)(TSF"bufReq.size:%_", _buffer_req.Length());
    send_body_.Attach(_buffer_req);
    send_extend_.Attach(_buffer_extend);

    runner_.Start();
}

void ShortlinkEmptyImpl::_StartInternal() {
    if (OnSend) {
        OnSend(this);
    } else {
        xwarn2(TSF"OnSend NULL.");
    }
    auto cb = shortlink_callback_bridge_.lock();
    if (cb) {
        std::string send_body((const char*)send_body_.Ptr(), send_body_.Length());
        std::string extend_body((const char*)send_extend_.Ptr(), send_extend_.Length());
        cb->OnSendData(send_body, extend_body);
        return;
    }
    NO_FIND_CALLBACK_ERROR(task_.taskid, task_.cmdid);
}

void ShortlinkEmptyImpl::OnReceiveDataFromCaller(const std::string& _data, int _status_code) {
    xinfo2(TSF"OnReceiveDataFromCaller %_, %_", _data.size(), _status_code);
    if (OnRecv) {
        OnRecv(this, _data.size(), _data.size());
    } else {
        xwarn2(TSF"OnRecv NULL.");
    }
    _OnResponse(_data, _status_code);
}

void ShortlinkEmptyImpl::OnReceiveErrorFromCaller(ErrCmdType _error_type, int _error_code) {
    _OnResponseError(_error_type, _error_code);
}

void ShortlinkEmptyImpl::_OnResponse(const std::string& data, int _status_code) {
    if (OnResponse) {
        AutoBuffer body;
        body.Write(data.c_str(), data.length());
        AutoBuffer extension;
        move_wrapper<AutoBuffer> m_body(body);
        move_wrapper<AutoBuffer> m_extension(extension);
        ConnectProfile profile;
        OnResponse(this, kEctOK, _status_code, m_body, m_extension, false, profile);
    } else {
        xwarn2(TSF"OnResponse NULL.");
    }
}

void ShortlinkEmptyImpl::_OnResponseError(ErrCmdType _error_type, int _error_code) {
    if (OnResponse) {
        AutoBuffer b;
        AutoBuffer e;
        move_wrapper<AutoBuffer> m_body(b);
        move_wrapper<AutoBuffer> m_extension(e);
        ConnectProfile profile;
        OnResponse(this, _error_type, _error_code, m_body, m_extension, false, profile);
    } else {
        xwarn2(TSF"OnResponse NULL.");
    }
}

}} //namespace mars stn
