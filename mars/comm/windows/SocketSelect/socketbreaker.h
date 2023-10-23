
#include "thread/lock.h"
#include "socket/unix_socket.h"

namespace mars {
namespace comm {

class SocketSelect;

class SocketBreaker {
    friend SocketSelect;

 public:
    SocketBreaker();
    ~SocketBreaker();

    bool IsCreateSuc() const;
    bool ReCreate();
    void Close();

    bool Break();
    bool Break(int reason);
    bool Clear();
    bool PreciseBreak(uint32_t cookie);
    bool PreciseClear(uint32_t* cookie);

    bool IsBreak() const;
    WSAEVENT BreakerFD() const;
    int BreakReason() const;

 private:
    SocketBreaker(const SocketBreaker&);
    SocketBreaker& operator=(const SocketBreaker&);

 private:
    Mutex m_mutex;
    WSAEVENT m_event;
    bool m_create_success;
    bool m_broken;
    int m_exception;
    int m_reason;
};
}  // namespace comm
}  // namespace mars
