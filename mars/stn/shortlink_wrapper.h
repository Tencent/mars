#ifndef STN_SHORTLINK_WRAPPER_H_
#define STN_SHORTLINK_WRAPPER_H_

#include <string>
#include "stn.h"

namespace mars {
namespace stn {
class ShortLinkInterface;

class ShortLinkWrapper {
public:
    ShortLinkWrapper() = default;
    ~ShortLinkWrapper() = default;

public:
    void SetShortLinkInterface(ShortLinkInterface* _interface);
    void OnReceiveDataFromCaller(const std::string& _data, int _status_code);
    void OnReceiveErrorFromCaller(ErrCmdType _error_type, int _error_code);

private:
    ShortLinkInterface* shortlink_;


};


}} // namespace mars stn



#endif // STN_SHORTLINK_WRAPPER_H_