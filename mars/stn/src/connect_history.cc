#include "connect_history.h"

#include <mutex>

const unsigned kMaxConnectHistoryCount = 20;

namespace mars {
namespace stn {
void ConnectHistory::AddRtt(const HistoryRtt& rtt) {
    std::lock_guard<std::mutex> lock(mutex_);
    lst_history_rtt_.remove_if([&](const HistoryRtt& history) {
        return history.connected_address_ == rtt.connected_address_
               && history.transport_protocol == rtt.transport_protocol;
    });
    lst_history_rtt_.emplace_back(rtt);
    if (lst_history_rtt_.size() > kMaxConnectHistoryCount) {
        lst_history_rtt_.pop_front();
    }
}
void ConnectHistory::Reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    lst_history_rtt_.clear();
}
bool ConnectHistory::GetRtt(const socket_address& address, int transport_protocol, struct HistoryRtt* outrtt) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto rit = lst_history_rtt_.rbegin(); rit != lst_history_rtt_.rend(); rit++) {
        if (rit->connected_address_ == address && rit->transport_protocol == transport_protocol) {
            *outrtt = *rit;
            return true;
        }
    }
    return false;
}
}  // namespace stn
}  // namespace mars