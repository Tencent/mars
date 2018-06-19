/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * SocketSelect.h
 *
 *  Created on: 2016-6-22
 *      Author: zhoudingping
 */



#include "socketselect.h"

#include <poll.h>

#include <algorithm>

#include "comm/xlogger/xlogger.h"


#if 0/*TARGET_OS_MAC*/
SocketSelect::SocketSelect(SocketBreaker& _breaker, bool _autoclear)
: breaker_(_breaker), kq_(0), events_(NULL), trigered_events_(0), errno_(0), autoclear_(_autoclear)
{
    // inital FD
}

SocketSelect::~SocketSelect()
{
    if (kq_) {
        close(kq_);
    }
    if (events_) {
        delete [] events_;
    }
}

const int EVENT_COUNT = 100;

void SocketSelect::PreSelect()
{
    if (kq_) {
        close(kq_);
    }
    if (events_) {
        delete [] events_;
    }

    kq_ = kqueue();
    trigered_events_ = 0;
    errno_ = 0;

    events_ = new struct kevent[EVENT_COUNT];
    memset(events_, 0, sizeof(struct kevent) * EVENT_COUNT);
}

int SocketSelect::Select()
{
    int ret = kevent(kq_, NULL, 0, events_, EVENT_COUNT, NULL);

    if (0 > ret) errno_ = errno;
    else if (ret > 0) trigered_events_ = ret;

    if (autoclear_) Breaker().Clear();

    return ret;
}

int SocketSelect::Select(int _msec)
{
    ASSERT(0 <= _msec);

    int sec = _msec / 1000;
    int usec = (_msec - sec * 1000) * 1000;
    timespec timeout = {sec, usec};

    int ret = kevent(kq_, NULL, 0, events_, EVENT_COUNT, &timeout);

    if (0 > ret) errno_ = errno;
    else if (ret > 0) trigered_events_ = ret;

    if (autoclear_) Breaker().Clear();

    return ret;
}

int SocketSelect::Select(int _sec, int _usec)
{
    ASSERT(0 <= _sec);
    ASSERT(0 <= _usec);
    timespec timeout = {_sec, _usec};

    int ret = kevent(kq_, NULL, 0, events_, EVENT_COUNT, &timeout);

    if (0 > ret) errno_ = errno;
    else if (ret > 0) trigered_events_ = ret;

    if (autoclear_) Breaker().Clear();

    return ret;
}

void SocketSelect::Read_FD_SET(int _socket)
{
    struct kevent changes;
    EV_SET(&changes, _socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq_, &changes, 1, NULL, 0, NULL);
}

void SocketSelect::Write_FD_SET(int _socket)
{
    struct kevent changes;
    EV_SET(&changes, _socket, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    kevent(kq_, &changes, 1, NULL, 0, NULL);
}

void SocketSelect::Exception_FD_SET(int _socket)
{
    //    struct kevent changes;
    //    EV_SET(&changes, _socket, EVFILT_READ|EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    //    kevent(kq_, &changes, 1, NULL, 0, NULL);
}

int SocketSelect::Read_FD_ISSET(int _socket) const
{
    for (int i = 0; i < trigered_events_; i++) {
        const struct kevent& ev = events_[i];
        if (ev.ident == _socket) {
            return 1;
        }
    }

    return 0;
}

int SocketSelect::Write_FD_ISSET(int _socket) const
{
    for (int i = 0; i < trigered_events_; i++) {
        const struct kevent& ev = events_[i];
        if (ev.ident == _socket) {
            return 1;
        }
    }

    return 0;
}

int SocketSelect::Exception_FD_ISSET(int _socket) const
{
    for (int i = 0; i < trigered_events_; i++) {
        const struct kevent& ev = events_[i];
        if (ev.ident == _socket && ev.flags & EV_ERROR) {
            return 1;
        }
    }

    return 0;
}

bool SocketSelect::IsException() const
{
    return Exception_FD_ISSET(breaker_.BreakerFD());
}

bool SocketSelect::IsBreak() const
{

    return Read_FD_ISSET(breaker_.BreakerFD());
}

SocketBreaker& SocketSelect::Breaker()
{
    return breaker_;
}

int SocketSelect::Errno() const
{
    return errno_;
}

#else

SocketSelect::SocketSelect(SocketBreaker& _breaker, bool _autoclear)
: socket_poll_(_breaker, _autoclear)
{}

SocketSelect::~SocketSelect() {}

void SocketSelect::PreSelect() { socket_poll_.ClearEvent(); }
int  SocketSelect::Select() { return Select(-1); }
int  SocketSelect::Select(int _msec) { return socket_poll_.Poll(_msec); }

void SocketSelect::Read_FD_SET(SOCKET _socket) { socket_poll_.ReadEvent(_socket, true); }
void SocketSelect::Write_FD_SET(SOCKET _socket) { socket_poll_.WriteEvent(_socket, true); }
void SocketSelect::Exception_FD_SET(SOCKET _socket) { socket_poll_.NullEvent(_socket); }

int SocketSelect::Read_FD_ISSET(SOCKET _socket) const {
    const std::vector<PollEvent>& events = socket_poll_.TriggeredEvents();
    auto find_it = std::find_if(events.begin(), events.end(), [_socket](const PollEvent& _v){ return _v.FD() == _socket; });
    if (find_it == events.end()) return 0;
    return find_it->Readable() || find_it->HangUp();
}

int SocketSelect::Write_FD_ISSET(SOCKET _socket) const {
    const std::vector<PollEvent>& events = socket_poll_.TriggeredEvents();
    auto find_it = std::find_if(events.begin(), events.end(), [_socket](const PollEvent& _v){ return _v.FD() == _socket; });
    if (find_it == events.end()) {
        return 0;
    }
    return find_it->Writealbe();
}

int SocketSelect::Exception_FD_ISSET(SOCKET _socket) const {
    const std::vector<PollEvent>& events = socket_poll_.TriggeredEvents();
    auto find_it = std::find_if(events.begin(), events.end(), [_socket](const PollEvent& _v){ return _v.FD() == _socket; });
    if (find_it == events.end()) return 0;
    return find_it->Error() || find_it->Invalid();
}

int  SocketSelect::Ret() const { return socket_poll_.Ret(); }
int  SocketSelect::Errno() const { return socket_poll_.Errno(); }
bool SocketSelect::IsException() const { return socket_poll_.BreakerIsError(); }
bool SocketSelect::IsBreak() const { return socket_poll_.BreakerIsBreak(); }

SocketBreaker& SocketSelect::Breaker() { return socket_poll_.Breaker(); }
SocketPoll& SocketSelect::Poll() { return socket_poll_; }

#endif

