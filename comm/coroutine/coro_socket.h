//
//

#ifndef MMNET_ASYNC_SOCKET_H_H
#define MMNET_ASYNC_SOCKET_H_H

#include "comm/tickcount.h"
#include "comm/socket/unix_socket.h"
#include "comm/socket/socketselect.h"
#include "comm/messagequeue/message_queue.h"

class socket_address;
class AutoBuffer;

namespace coroutine {

SOCKET  block_socket_connect(const socket_address& _address, SocketSelectBreaker& _breaker, int& _errcode, int32_t _timeout=-1/*ms*/);
int     block_socket_send(SOCKET _sock, const void* _buffer, size_t _len, SocketSelectBreaker& _breaker, int &_errcode, int _timeout=-1);
int     block_socket_recv(SOCKET _sock, AutoBuffer& _buffer, size_t _max_size, SocketSelectBreaker& _breaker, int &_errcode, int _timeout=-1, bool _wait_full_size=false);

    
class SocketSelect : public ::SocketSelect {
public:
    SocketSelect(SocketSelectBreaker& _breaker);
    ~SocketSelect() {}
    
    virtual int Select(int _msec);
    virtual int Select() { return Select(-1);}
    
private:
    SocketSelect(const SocketSelect&);
    SocketSelect& operator=(const SocketSelect&);
    
public:
    tickcount_t  abs_timeout_;
};
    
class CoroutineSelect;
class TaskInfo;
class RunloopCond : public mq::RunloopCond {
public:
    RunloopCond();
    virtual ~RunloopCond();
    
public:
    void Select(TaskInfo& _task);
    
public:
    virtual const boost::typeindex::type_info& type() const;
    virtual void  Wait(ScopedLock& _lock, long _millisecond);
    virtual void  Notify(ScopedLock& _lock);
    
private:
    RunloopCond(const RunloopCond&);
    void operator=(const RunloopCond&);
    
private:
    CoroutineSelect* coroutine_select;
};
    
}

#define COMPLEX_CONNECT_NAMESPACE coroutine
#include "comm/socket/complexconnect.h"

#endif //MMNET_ASYNC_SOCKET_H_H
