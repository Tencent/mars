#include "shortlink_wrapper.h"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/src/shortlink_interface.h"

namespace mars {
namespace stn {

void ShortLinkWrapper::SetShortLinkInterface(ShortLinkInterface* _interface) {
    shortlink_ = _interface;
}
void ShortLinkWrapper::OnReceiveDataFromCaller(const std::string& _data, int _status_code) {
    if (shortlink_) {
        shortlink_->OnReceiveDataFromCaller(_data, _status_code);
        return;
    }
    xerror2(TSF"no find shortlink interface");
}
void ShortLinkWrapper::OnReceiveErrorFromCaller(ErrCmdType _error_type, int _error_code) {
    if (shortlink_) {
        shortlink_->OnReceiveErrorFromCaller(_error_type, _error_code);
        return;
    }
    xerror2(TSF"no find shortlink interface");
}

}} //namespace mars stn
