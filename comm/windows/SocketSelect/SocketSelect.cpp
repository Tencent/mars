/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
#include "SocketSelect.h"
#include <ws2tcpip.h>

#include "comm/xlogger/xlogger.h"
#include "socket/unix_socket.h"


SocketSelectBreaker::SocketSelectBreaker()
    : m_sendinlen(sizeof(m_sendin))
    , m_socket_w(INVALID_SOCKET)
    , m_socket_r(INVALID_SOCKET)
    , m_broken(false)
    , m_create_success(true) {
    ReCreate();
}

SocketSelectBreaker::~SocketSelectBreaker() {
    Close();
}

bool SocketSelectBreaker::IsCreateSuc() const {
    return m_create_success;
}

bool SocketSelectBreaker::ReCreate() {
    Close();

    // initial pipes
    int Ret;
    m_socket_w = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (INVALID_SOCKET == m_socket_w) {
        xerror2("INVALID_SOCKET with  m_socket_w=%d", WSAGetLastError());
        m_create_success = false;
        return m_create_success;
    }

    m_socket_r = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (INVALID_SOCKET == m_socket_r) {
        xerror2("INVALID_SOCKET with m_socket_r=%d", WSAGetLastError());
        socket_close(m_socket_w);
        m_create_success = false;
        return m_create_success;
    }

    Ret = socket_set_nobio(m_socket_w);

    if (SOCKET_ERROR == Ret) {
        xerror2("ioctlsocket failed with m_socket_w: %d\n", WSAGetLastError());
        socket_close(m_socket_r);
        socket_close(m_socket_w);
        m_create_success = false;
        return m_create_success;
    }

    Ret = socket_set_nobio(m_socket_r);

    if (SOCKET_ERROR == Ret) {
        xerror2("ioctlsocket failed with m_socket_r: %d\n", WSAGetLastError());
        socket_close(m_socket_r);
        socket_close(m_socket_w);
        m_create_success = false;
        return m_create_success;
    }

    struct sockaddr_in local_b;

    local_b.sin_family = AF_INET;

    local_b.sin_port = htons(0);

    local_b.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);

    Ret = bind(m_socket_w, (struct sockaddr*)&local_b, sizeof(local_b));

    if (SOCKET_ERROR == Ret) {
        xerror2("bind failed with m_socket_w: %d\n", WSAGetLastError());
        socket_close(m_socket_r);
        socket_close(m_socket_w);
        m_create_success = false;
        return m_create_success;
    }

    Ret = bind(m_socket_r, (struct sockaddr*)&local_b, sizeof(local_b));

    if (SOCKET_ERROR == Ret) {
        xerror2("bind failed with m_socket_r: %d\n", WSAGetLastError());
        socket_close(m_socket_r);
        socket_close(m_socket_w);
        m_create_success = false;
        return m_create_success;
    }

    Ret = getsockname(m_socket_r, &m_sendin, &m_sendinlen);

    if (SOCKET_ERROR == Ret) {
        xerror2("getsockname failed %d\n", WSAGetLastError());
        socket_close(m_socket_r);
        socket_close(m_socket_w);
        m_create_success = false;
        return m_create_success;
    }

    m_create_success = true;
    return m_create_success;
}

bool SocketSelectBreaker::IsBreak() const {
    return m_broken;
}

bool SocketSelectBreaker::Break() {
    ScopedLock lock(m_mutex);

    if (m_broken) return true;

    char dummy[] = "1";
    int ret = sendto(m_socket_w, &dummy, strlen(dummy), 0, (sockaddr*)&m_sendin, m_sendinlen);
    m_broken = true;

    if (ret < 0 || ret != strlen(dummy)) {
        xerror2(TSF"sendto Ret:%_, errno:(%_, %_)", ret, errno, WSAGetLastError());
        m_broken =  false;
        ReCreate();
    }

    // Ret = WSAGetLastError();
    return m_broken;
}



bool SocketSelectBreaker::Clear() {
    ScopedLock lock(m_mutex);

    if (!m_broken) return true;

    char buf[128];
    struct sockaddr src = {0};
    int len = sizeof(src);
    int ret = recvfrom(m_socket_r, buf, sizeof(buf), 0, &src, &len);
    m_broken = false;

    if (ret < 0) {
        xerror2(TSF"recvfrom Ret:%_, errno:(%_, %_)", ret, errno, WSAGetLastError());
        ReCreate();
        return false;
    }

    // Ret = WSAGetLastError();
    return true;
}

void SocketSelectBreaker::Close() {
    if (m_socket_w != INVALID_SOCKET)
        socket_close(m_socket_w);

    if (m_socket_r != INVALID_SOCKET)
        socket_close(m_socket_r);

    m_socket_w = INVALID_SOCKET;
    m_socket_r = INVALID_SOCKET;
}

SOCKET SocketSelectBreaker::BreakerFD() const {
    return m_socket_r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SocketSelect::SocketSelect(SocketSelectBreaker& _breaker, bool _autoclear)
    : breaker_(_breaker), maxsocket_(0), errno_(0), autoclear_(_autoclear) {
    // inital FD
    FD_ZERO(&readfd_);
    FD_ZERO(&writefd_);
    FD_ZERO(&exceptionfd_);
}

SocketSelect::~SocketSelect()
{}

void SocketSelect::PreSelect() {
    xassert2(!IsBreak(), "Already break!");
    FD_ZERO(&readfd_);
    FD_ZERO(&writefd_);
    FD_ZERO(&exceptionfd_);

    FD_SET(breaker_.BreakerFD(), &readfd_);
    FD_SET(breaker_.BreakerFD(), &exceptionfd_);
    maxsocket_ = breaker_.BreakerFD();
    errno_ = 0;
}

int SocketSelect::Select() {
    int ret = select(maxsocket_ + 1, &readfd_, &writefd_, &exceptionfd_, NULL);

    if (0 > ret) errno_ = errno;

    if (autoclear_) Breaker().Clear();

    return ret;
}

int SocketSelect::Select(int _msec) {
    ASSERT(0 <= _msec);

    int sec = _msec / 1000;
    int usec = (_msec - sec * 1000) * 1000;
    timeval timeout = {sec, usec};
    int ret = select(maxsocket_ + 1, &readfd_, &writefd_, &exceptionfd_, &timeout);

    if (0 > ret) errno_ = errno;

    if (autoclear_) Breaker().Clear();

    return ret;
}

int SocketSelect::Select(int _sec, int _usec) {
    ASSERT(0 <= _sec);
    ASSERT(0 <= _usec);
    timeval timeout = {_sec, _usec};
    int ret = select(maxsocket_ + 1, &readfd_, &writefd_, &exceptionfd_, &timeout);

    if (0 > ret) errno_ = errno;

    if (autoclear_) Breaker().Clear();

    return ret;
}

void SocketSelect::Read_FD_SET(SOCKET _socket) {
    maxsocket_ = std::max(maxsocket_, _socket);
    FD_SET(_socket, &readfd_);
}

void SocketSelect::Write_FD_SET(SOCKET _socket) {
    maxsocket_ = std::max(maxsocket_, _socket);
    FD_SET(_socket, &writefd_);
}

void SocketSelect::Exception_FD_SET(SOCKET _socket) {
    maxsocket_ = std::max(maxsocket_, _socket);
    FD_SET(_socket, &exceptionfd_);
}

int SocketSelect::Read_FD_ISSET(SOCKET _socket) const {
    return FD_ISSET(_socket, &readfd_);
}

int SocketSelect::Write_FD_ISSET(SOCKET _socket) const {
    return FD_ISSET(_socket, &writefd_);
}

int SocketSelect::Exception_FD_ISSET(SOCKET _socket) const {
    return FD_ISSET(_socket, &exceptionfd_);
}

bool SocketSelect::IsException() const {
    return 0 != FD_ISSET(breaker_.BreakerFD(), &exceptionfd_);
}

bool SocketSelect::IsBreak() const {
    return 0 != FD_ISSET(breaker_.BreakerFD(), &readfd_);
}

SocketSelectBreaker& SocketSelect::Breaker() {
    return breaker_;
}

int SocketSelect::Errno() const {
    return errno_;
}

