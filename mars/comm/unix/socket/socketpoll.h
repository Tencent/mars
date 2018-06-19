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
 * SocketPoll.h
 *
 *  Created on: 2015-12-9
 *      Author: yerungui
 */


#ifndef _SOCKSTPOLL_
#define _SOCKSTPOLL_ 

#include <poll.h>

#include <vector>
#include <map>

#include "comm/socket/unix_socket.h"
#include "comm/socket/socketbreaker.h"

struct PollEvent {
    friend class SocketPoll;
public:
    PollEvent();
    
    bool  Readable() const;
    bool  Writealbe() const;
    bool  HangUp() const;
    bool  Error() const;
    bool  Invalid() const;
    
    void* UserData();
    SOCKET FD() const;
    
private:
    pollfd    poll_event_;
    void*     user_data_;
};

class SocketPoll {
public:
    SocketPoll(SocketBreaker& _breaker, bool _autoclear = false);
    virtual ~SocketPoll();
    
    bool Consign(SocketPoll& _consignor, bool _recover = false);
    void AddEvent(SOCKET _fd, bool _read, bool _write, void* _user_data);
    void ReadEvent(SOCKET _fd, bool _active);
    void WriteEvent(SOCKET _fd, bool _active);
    void NullEvent(SOCKET _fd);
    void DelEvent(SOCKET _fd);
    void ClearEvent();

    virtual int Poll();
    virtual int Poll(int _msec);
    
    int  Ret() const;
    int  Errno() const;
    bool BreakerIsBreak() const;
    bool BreakerIsError() const;
    
    bool ConsignReport(SocketPoll& _consignor, int64_t _timeout) const;
    const std::vector<PollEvent>& TriggeredEvents() const;
    
    SocketBreaker& Breaker();
    
private:
    SocketPoll(const SocketPoll&);
    SocketPoll& operator=(const SocketPoll&);
    
protected:
    SocketBreaker&       breaker_;
    const bool           autoclear_;
    
    std::vector<pollfd>         events_;
    std::map<SOCKET, void*>     events_user_data_;
    std::vector<PollEvent>      triggered_events_;
    
    int                    ret_;
    int                    errno_;
};

#endif
