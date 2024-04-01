#include "socketbreaker.h"
#include "xlogger/xlogger.h"

namespace mars {
namespace comm {

SocketBreaker::SocketBreaker() : m_broken(false), m_create_success(true), m_exception(0) {
    ReCreate();
}

SocketBreaker::~SocketBreaker() {
    Close();
}

bool SocketBreaker::IsCreateSuc() const {
    return m_create_success;
}

bool SocketBreaker::ReCreate() {
    m_event = WSACreateEvent();
    m_create_success = WSA_INVALID_EVENT != m_event;
    ASSERT2(m_create_success, "%d, %s", WSAGetLastError(), gai_strerror(WSAGetLastError()));
    m_exception = WSAGetLastError();
    m_broken = !m_create_success;
    return m_create_success;
}

bool SocketBreaker::IsBreak() const {
    return m_broken;
}

bool SocketBreaker::Break() {
    ScopedLock lock(m_mutex);
    bool ret = WSASetEvent(m_event);
    xerror2_if(!ret, TSF "%_, %_", WSAGetLastError(), gai_strerror(WSAGetLastError()));
    m_broken = ret;

    if (!ret)
        m_exception = WSAGetLastError();

    return m_broken;
}

bool SocketBreaker::Break(int reason) {
    m_reason = reason;
    return Break();
}

bool SocketBreaker::Clear() {
    ScopedLock lock(m_mutex);

    if (!m_broken)
        return true;

    bool ret = WSAResetEvent(m_event);
    ASSERT2(ret, "%d, %s", WSAGetLastError(), gai_strerror(WSAGetLastError()));

    m_broken = !ret;

    if (!ret)
        m_exception = WSAGetLastError();

    return ret;
}

// WSASetEvent无法向fd中写入cookie
bool SocketBreaker::PreciseBreak(uint32_t cookie) {
    ScopedLock lock(m_mutex);
    bool ret = WSASetEvent(m_event);
    ASSERT2(ret, "%d, %s", WSAGetLastError(), gai_strerror(WSAGetLastError()));
    m_broken = ret;

    if (!ret)
        m_exception = WSAGetLastError();

    return m_broken;
}

// WSAResetEvent无法读取fd中的cookie
bool SocketBreaker::PreciseClear(uint32_t* cookie) {
    ScopedLock lock(m_mutex);

    if (!m_broken)
        return true;

    bool ret = WSAResetEvent(m_event);
    ASSERT2(ret, "%d, %s", WSAGetLastError(), gai_strerror(WSAGetLastError()));

    m_broken = !ret;

    if (!ret)
        m_exception = WSAGetLastError();

    return ret;
}

void SocketBreaker::Close() {
    bool ret = WSACloseEvent(m_event);
    ASSERT2(ret, "%d, %s", WSAGetLastError(), gai_strerror(WSAGetLastError()));
    m_exception = WSAGetLastError();
    m_event = WSA_INVALID_EVENT;
    m_broken = true;
}

WSAEVENT SocketBreaker::BreakerFD() const {
    return m_event;
}

int SocketBreaker::BreakReason() const {
    return m_reason;
}
}  // namespace comm
}  // namespace mars
