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
 * SocketPoll.cpp
 *
 *  Created on: 2015-12-9
 *      Author: yerungui
 */

#include "socketpoll.h"

#include <algorithm>

#include "comm/xlogger/xlogger.h"


PollEvent::PollEvent():poll_event_({0}), user_data_(NULL) { }
bool  PollEvent::Readable() const { return poll_event_.revents & POLLIN; }
bool  PollEvent::Writealbe() const { return poll_event_.revents & POLLOUT; }
bool  PollEvent::HangUp() const { return poll_event_.revents & POLLHUP; }
bool  PollEvent::Error() const { return poll_event_.revents & POLLERR; }
bool  PollEvent::Invalid() const  { return poll_event_.revents & POLLNVAL; }
void* PollEvent::UserData() { return user_data_;}
SOCKET PollEvent::FD() const { return poll_event_.fd; }
    
//////////////////////////////////////////////

SocketPoll::SocketPoll(SocketBreaker& _breaker, bool _autoclear)
: breaker_(_breaker), autoclear_(_autoclear), ret_(0), errno_(0)
{
    events_.push_back({breaker_.BreakerFD(), POLLIN, 0});
}

SocketPoll::~SocketPoll() {}

bool SocketPoll::Consign(SocketPoll& _consignor, bool _recover) {
    auto it = std::find_if(events_.begin(), events_.end(), [&_consignor](const pollfd& _v){ return _v.fd == _consignor.events_[0].fd;});
    
    if (_recover) {
        if (it == events_.end()) return false;
        xassert2(it->events == _consignor.events_[0].events, TSF"%_ != %_", it->events, _consignor.events_[0].events);
        events_.erase(it, it+_consignor.events_.size());
    } else {
        xassert2(it == events_.end());
        if (it != events_.end()) return false;
        events_.insert(events_.end(), _consignor.events_.begin(), _consignor.events_.end());
    }
    
    return true;
}

void SocketPoll::AddEvent(SOCKET _fd, bool _read, bool _write, void* _user_data) {
    
    auto it = std::find_if(events_.begin(), events_.end(), [&_fd](const pollfd& _v){ return _v.fd == _fd;});
    pollfd add_event = {_fd, static_cast<short>((_read? POLLIN:0) | (_write? POLLOUT:0)), 0};
    if (it == events_.end()) {
        events_.push_back(add_event);
    } else {
        *it = add_event;
    }
    events_user_data_[_fd] = _user_data;
}

void SocketPoll::ReadEvent(SOCKET _fd, bool _active) {
    
    auto find_it = std::find_if(events_.begin(), events_.end(), [&_fd](const pollfd& _v){ return _v.fd == _fd;});
    if (find_it == events_.end()) {
        AddEvent(_fd, _active?true:false, false, NULL);
        return;
    }
    
    if (_active)
        find_it->events |= POLLIN;
    else
        find_it->events &= ~POLLIN;
}

void SocketPoll::WriteEvent(SOCKET _fd, bool _active) {
    
    auto find_it = std::find_if(events_.begin(), events_.end(), [&_fd](const pollfd& _v){ return _v.fd == _fd;});
    if (find_it == events_.end()) {
        AddEvent(_fd, false, _active?true:false, NULL);
        return;
    } 
    
    if (_active)
        find_it->events |= POLLOUT;
    else
        find_it->events &= ~POLLOUT;
}

void SocketPoll::NullEvent(SOCKET _fd) {
    auto find_it = std::find_if(events_.begin(), events_.end(), [&_fd](const pollfd& _v){ return _v.fd == _fd;});
    if (find_it == events_.end()) {
        AddEvent(_fd, false, false, NULL);
    }
}

void SocketPoll::DelEvent(SOCKET _fd) {
    auto find_it = std::find_if(events_.begin(), events_.end(), [&_fd](const pollfd& _v){ return _v.fd == _fd;});
    if (find_it != events_.end()) events_.erase(find_it);
    events_user_data_.erase(_fd);
}

void SocketPoll::ClearEvent() {
    events_.erase(events_.begin()+1, events_.end());
    events_user_data_.clear();
}

int SocketPoll::Poll() { return Poll(-1); }

int SocketPoll::Poll(int _msec) {
    ASSERT(-1 <= _msec);
    if (-1 > _msec) _msec = 0;
    
    triggered_events_.clear();
    errno_ = 0;
    ret_   = 0;
    for (auto &i : events_) { i.revents = 0; }
    
    ret_ = poll(&events_[0], (nfds_t)events_.size(), _msec);
    
    do {
        
        if (0 > ret_) {
            errno_ = errno;
            break;
        }
        
        if (0 == ret_) {
            break;
        }
        
        for (size_t i = 1; i < events_.size(); ++i) {
            if (0 == events_[i].revents ) continue;
            
            PollEvent traggered_event;
            traggered_event.poll_event_ = events_[i];
            traggered_event.user_data_  = events_user_data_[events_[i].fd];
            
            triggered_events_.push_back(traggered_event);
        }
    } while(false);
    
    if (autoclear_) Breaker().Clear();
    return ret_;
}

int SocketPoll::Ret() const { return ret_; }
int SocketPoll::Errno() const { return errno_; }

bool SocketPoll::BreakerIsError() const {
    PollEvent logic_event;
    logic_event.poll_event_ = events_[0];
    return logic_event.Invalid() || logic_event.Error();
}

bool SocketPoll::BreakerIsBreak() const {
    PollEvent logic_event;
    logic_event.poll_event_ = events_[0];;
    return logic_event.Readable();
}

bool SocketPoll::ConsignReport(SocketPoll& _consignor, int64_t _timeout) const {
    
    int32_t triggered_event_count = 0;
    auto find_it = std::find_if(events_.begin(), events_.end(), [&_consignor](const pollfd _v){ return _v.fd == _consignor.events_[0].fd;});
    
    xassert2(find_it != events_.end());
    xassert2(events_.end() - find_it >= (int)_consignor.events_.size());
    
    if (find_it == events_.end()) return false;
    
    for (auto &i : _consignor.events_) {
        xassert2(i.fd == find_it->fd && i.events == find_it->events,
                 TSF"i(%_, %_), find_it(%_, %_)", i.fd, i.events, find_it->fd, find_it->events);
        if (0 != find_it->revents) {
            i.revents = find_it->revents;
            ++triggered_event_count;
            
            if (i.fd == _consignor.events_[0].fd) {
                xassert2(&i == &(_consignor.events_[0]));
                continue;
            }
            
            PollEvent traggered_event;
            traggered_event.poll_event_ = i;
            traggered_event.user_data_  = _consignor.events_user_data_[i.fd];
            
            _consignor.triggered_events_.push_back(traggered_event);
        }
        ++find_it;
    }
    
    if (0 > ret_) {
        _consignor.ret_   = ret_;
        _consignor.errno_ = errno_;
        if (_consignor.autoclear_) _consignor.Breaker().Clear();
        return true;
    }
    
    if (0 < triggered_event_count) {
        _consignor.ret_   = triggered_event_count;
        _consignor.errno_ = 0;
        if (_consignor.autoclear_) _consignor.Breaker().Clear();
        return true;
    }
    
    if (0 >= _timeout) {
        _consignor.ret_   = 0;
        _consignor.errno_ = errno_;
        if (_consignor.autoclear_) _consignor.Breaker().Clear();
        return true;
    }
    
    return false;
}

const std::vector<PollEvent>& SocketPoll::TriggeredEvents() const {
    return triggered_events_;
}

SocketBreaker& SocketPoll::Breaker() {
    return breaker_;
}

