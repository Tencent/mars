// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * SocketSelect.h
 *
 *  Created on: 2016-6-22
 *      Author: zhoudingpin
 */



#include "socketselect.h"

#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

#include "mars/comm/xlogger/xlogger.h"


SocketSelectBreaker::SocketSelectBreaker()
: create_success_(true),
broken_(false) {
    ReCreate();
}

SocketSelectBreaker::~SocketSelectBreaker() {
    Close();
}

bool SocketSelectBreaker::IsCreateSuc() const {
    return create_success_;
}

bool SocketSelectBreaker::ReCreate() {
    pipes_[0] = -1;
    pipes_[1] = -1;

    int Ret;
    Ret = pipe(pipes_);
    xassert2(-1 != Ret, "pipe errno=%d", errno);

    if (Ret == -1) {
        pipes_[0] = -1;
        pipes_[1] = -1;
        create_success_ = false;
        return create_success_;
    }

    long flags0 = fcntl(pipes_[0], F_GETFL, 0);
    long flags1 = fcntl(pipes_[1], F_GETFL, 0);

    if (flags0 < 0 || flags1 < 0) {
        xerror2(TSF"get old flags error");
        close(pipes_[0]);
        close(pipes_[1]);
        pipes_[0] = -1;
        pipes_[1] = -1;
        create_success_ = false;
        return create_success_;
    }

    flags0 |= O_NONBLOCK;
    flags1 |= O_NONBLOCK;
    int ret0 = fcntl(pipes_[0], F_SETFL, flags0);
    int ret1 = fcntl(pipes_[1], F_SETFL, flags1);

    if ((-1 == ret1) || (-1 == ret0)) {
        xerror2(TSF"fcntl error");
        close(pipes_[0]);
        close(pipes_[1]);
        pipes_[0] = -1;
        pipes_[1] = -1;
        create_success_ = false;
        return create_success_;
    }

    create_success_ = true;
    return create_success_;
}

bool SocketSelectBreaker::Break() {
    ScopedLock lock(mutex_);

    if (broken_) return true;

    char dummy[] = "1";
    int ret = (int)write(pipes_[1], dummy, strlen(dummy));
    broken_ = true;

    if (ret < 0 || ret != (int)strlen(dummy))
    {
        xerror2(TSF"Ret:%_, errno:(%_, %_)", ret, errno, strerror(errno));
        broken_ =  false;
    }

    return broken_;
}

bool SocketSelectBreaker::Clear() {
    ScopedLock lock(mutex_);
    char dummy[128];
    int ret = (int)read(pipes_[0], dummy, sizeof(dummy));

    if (ret < 0) {
        xverbose2(TSF"Ret=%0", ret);
        return false;
    }

    broken_ =  false;
    return true;
}

void SocketSelectBreaker::Close() {
    broken_ =  true;
    if (pipes_[1] >= 0)
        close(pipes_[1]);
    if (pipes_[0] >= 0)
        close(pipes_[0]);
}

int SocketSelectBreaker::BreakerFD() const {
    return pipes_[0];
}

bool SocketSelectBreaker::IsBreak() const {
    return broken_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if 0/*TARGET_OS_MAC*/
SocketSelect::SocketSelect(SocketSelectBreaker& _breaker, bool _autoclear)
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

SocketSelectBreaker& SocketSelect::Breaker()
{
    return breaker_;
}

int SocketSelect::Errno() const
{
    return errno_;
}

#else

SocketSelect::SocketSelect(SocketSelectBreaker& _breaker, bool _autoclear)
: breaker_(_breaker), ret_(0), errno_(0), autoclear_(_autoclear)
{}

SocketSelect::~SocketSelect() {}

void SocketSelect::PreSelect() {
    vfds_.clear();

    int fd = breaker_.BreakerFD();
    struct pollfd fditem = {0};
    fditem.fd = fd;
    fditem.events = POLLIN|POLLPRI|POLLERR;
    vfds_.push_back(fditem);

    ret_ = 0;
    errno_ = 0;
}

void SocketSelect::Consign(SocketSelect& _consignor) {
    int fd = _consignor.Breaker().BreakerFD();
    struct pollfd fditem = {0};
    fditem.fd = fd;
    fditem.events = POLLIN|POLLPRI|POLLERR;
    vfds_.push_back(fditem);
    
    vfds_.insert(vfds_.end(), _consignor.vfds_.begin(), _consignor.vfds_.end());
}

int SocketSelect::Select() {
    return Select(-1);
}

int SocketSelect::Select(int _msec) {
    ret_ = poll(&vfds_[0], (nfds_t)vfds_.size(), _msec);
    if (0 > ret_) { errno_ = errno; }
    
    if (autoclear_) Breaker().Clear();
    
    return ret_;
}

bool SocketSelect::Report(SocketSelect& _consignor, int64_t _timeout) {
    int event_count = _consignor.Breaker().IsBreak()? 1:0;
    for (auto& x: vfds_) {
        for (auto& y: _consignor.vfds_) {
            xassert2(x.fd !=y.fd || (x.fd == y.fd && x.events == y.events), TSF"x(%_, %_), y(%_, %_)", x.fd, x.events, y.fd, y.events);
            if (x.fd == y.fd && x.events == y.events && 0 != x.revents) {
                y.revents = x.revents;
                ++event_count;
            }
        }
    }
    
    if (0 > ret_) {
        _consignor.ret_   = ret_;
        _consignor.errno_ = errno_;
        if (_consignor.autoclear_) _consignor.Breaker().Clear();
        return true;
    }
    
    if (0 == ret_ && 0 >= _timeout) {
        _consignor.ret_   = 0;
        _consignor.errno_ = errno_;
        if (_consignor.autoclear_) _consignor.Breaker().Clear();
        return true;
    }
    
    if (0 < event_count) {
        _consignor.ret_   = 0<ret_? ret_:1;
        _consignor.errno_ = errno_;
        if (_consignor.autoclear_) _consignor.Breaker().Clear();
        return true;
    }
    
    return false;
}

void SocketSelect::Read_FD_SET(int _socket) {
    for (size_t i = 0; i < vfds_.size(); i++){
        if (vfds_[i].fd == _socket){
            vfds_[i].events |= POLLIN|POLLERR;
            return;
        }
    }

    struct pollfd fditem = {0};
    fditem.fd = _socket;
    fditem.events = POLLIN|POLLERR;

    vfds_.push_back(fditem);
}

void SocketSelect::Write_FD_SET(int _socket) {
    for (size_t i = 0; i < vfds_.size(); i++){
        if (vfds_[i].fd == _socket){
            vfds_[i].events |= POLLOUT|POLLERR;
            return;
        }
    }

    struct pollfd fditem = {0};
    fditem.fd = _socket;
    fditem.events = POLLOUT|POLLERR;

    vfds_.push_back(fditem);
}

void SocketSelect::Exception_FD_SET(int _socket) {
    for (size_t i = 0; i < vfds_.size(); i++){
        if (vfds_[i].fd == _socket){
            vfds_[i].events |= POLLERR;
            return;
        }
    }

    struct pollfd fditem = {0};
    fditem.fd = _socket;
    fditem.events = POLLERR;

    vfds_.push_back(fditem);
}

int SocketSelect::Read_FD_ISSET(int _socket) const {
    for (size_t i = 0; i < vfds_.size(); i++){
        if (vfds_[i].fd == _socket)
            return vfds_[i].revents & (POLLIN|POLLHUP);
    }

    return 0;
}

int SocketSelect::Write_FD_ISSET(int _socket) const {
    for (size_t i = 0; i < vfds_.size(); i++){
        if (vfds_[i].fd == _socket)
            return vfds_[i].revents & (POLLOUT);
    }

    return 0;
}

int SocketSelect::Exception_FD_ISSET(int _socket) const {
    for (size_t i = 0; i < vfds_.size(); i++){
        if (vfds_[i].fd == _socket)
            return vfds_[i].revents & (POLLERR|POLLNVAL);
    }

    return 0;
}

bool SocketSelect::IsException() const {
    int breakfd = breaker_.BreakerFD();
    return Exception_FD_ISSET(breakfd);
}

bool SocketSelect::IsBreak() const {
    int breakfd = breaker_.BreakerFD();

    for (size_t i = 0; i < vfds_.size(); i++){
        if (vfds_[i].fd == breakfd)
            return vfds_[i].revents & POLLIN;
    }

    return 0;
}

SocketSelectBreaker& SocketSelect::Breaker() {
    return breaker_;
}

int SocketSelect::Errno() const {
    return errno_;
}
#endif

