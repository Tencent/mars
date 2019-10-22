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
 * socket_pool.h
 *
 *  Created on: 2019-07-24
 *      Author: zhouzhijie
 */

#ifndef SOCKET_POOL_
#define SOCKET_POOL_

#include "mars/stn/stn.h"
#include "mars/comm/tickcount.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/tickcount.h"

namespace mars {
namespace stn {

#define BAN_INTERVAL (5*60*1000)

    class CacheSocketItem {
    public:
        CacheSocketItem(const IPPortItem& _item, SOCKET _fd, uint32_t _timeout)
            :address_info(_item),start_tick(true), socket_fd(_fd), timeout(_timeout) {}

        bool IsSame(const IPPortItem& _item) const {
            return (_item.str_ip == address_info.str_ip && _item.port == address_info.port
                && _item.str_host == address_info.str_host);
        }

        bool HasTimeout() const {
            return start_tick.gettickspan() >= (timeout*1000);
        }

        IPPortItem address_info;
        tickcount_t start_tick;
        SOCKET socket_fd;
        uint32_t timeout;   //in seconds
    };

    class SocketPool {
    public:
        const int DEFAULT_MAX_KEEPALIVE_TIME = 5*1000;      //same as apache default

        SocketPool():use_cache_(true), is_baned_(false) {}
        ~SocketPool() {
            Clear();
        }

        SOCKET GetSocket(const IPPortItem& _item) {
            xverbose_function();
            ScopedLock lock(mutex_);
            if(!use_cache_ || _isBaned() || socket_pool_.empty())
                return INVALID_SOCKET;

            auto iter = socket_pool_.begin();
            while(iter != socket_pool_.end()) {
                if(iter->IsSame(_item)) {
                    if(iter->HasTimeout() || _IsSocketClosed(iter->socket_fd)) {
                        xinfo2(TSF"remove timeout or closed socket, is timeout:%_", iter->HasTimeout());
                        socket_close(iter->socket_fd);
                        iter = socket_pool_.erase(iter);
                        continue;
                    }
                    SOCKET fd = iter->socket_fd;
                    socket_pool_.erase(iter);
                    xinfo2(TSF"get from cache: ip:%_, port:%_, host:%_, fd:%_, size:%_", _item.str_ip, _item.port, _item.str_host, fd, socket_pool_.size());
                    return fd;
                }
                iter++;
            }
            xinfo2(TSF"can not find socket ip:%_, port:%_, host:%_, size:%_", _item.str_ip, _item.port, _item.str_host, socket_pool_.size());
            return INVALID_SOCKET;
        }

        bool AddCache(CacheSocketItem& item) {
            ScopedLock lock(mutex_);
            xinfo2(TSF"add item to socket pool, ip:%_, port:%_, host:%_, fd:%_, size:%_", item.address_info.str_ip, item.address_info.port, item.address_info.str_host, item.socket_fd, socket_pool_.size());
            socket_pool_.push_back(item);
            return true;
        }

        void CleanTimeout() {
            ScopedLock lock(mutex_);
            if(socket_pool_.empty())    return;
            
            auto iter = socket_pool_.begin();
            while(iter != socket_pool_.end()) {
                if(iter->HasTimeout()) {
                    socket_close(iter->socket_fd);
                    xinfo2(TSF"remove timeout socket: ip:%_, port:%_, host:%_, fd:%_", iter->address_info.str_ip, iter->address_info.port, iter->address_info.str_host, iter->socket_fd);
                    iter = socket_pool_.erase(iter);
                    continue;
                }
                iter++;
            }
            xinfo2(TSF"after clean, size:%_", socket_pool_.size());
        }

        void Clear() {
            ScopedLock lock(mutex_);
            xinfo2(TSF"clear cache sockets");
            std::for_each(socket_pool_.begin(), socket_pool_.end(), [](CacheSocketItem& value) {
                if(value.socket_fd != INVALID_SOCKET)
                    socket_close(value.socket_fd);
            });
            socket_pool_.clear();
        }

        void Report(bool _is_reused, bool _has_received, bool _is_decode_ok) {
            if(_is_reused && (!_has_received || !_is_decode_ok)) {
                ban_start_tick_.gettickcount();
                is_baned_ = true;
            } else if(_is_reused && _has_received && _is_decode_ok) {
                is_baned_ = false;
            }
        }
        
    private:
        bool _isBaned() {
            return is_baned_ && ban_start_tick_.isValid() && ban_start_tick_.gettickspan() <= BAN_INTERVAL;
        }

        bool _IsSocketClosed(SOCKET fd) {
            char buff[2];
            int try_count = 0;
            while(try_count < 2) {
                try_count++;
#ifndef WIN32
                ssize_t nrecv = ::recv(fd, buff, 1, MSG_PEEK|MSG_DONTWAIT);
#else
                ssize_t nrecv = ::recv(fd, buff, 1, MSG_PEEK);
#endif
                if (0 == nrecv) {
                    xerror2(TSF"socket already closed");
                    return true;
                }
                
                if (0 > nrecv && !IS_NOBLOCK_READ_ERRNO(socket_errno)) {
                    xerror2(TSF"socket error:(%_, %_)", socket_errno, strerror(socket_errno));
                    return true;
                } else {
                    return false;
                }
            }
            
            return true;
        }

    private:
        Mutex mutex_;
        bool use_cache_;
        std::vector<CacheSocketItem> socket_pool_;
        bool is_baned_;
        tickcount_t ban_start_tick_;
    };

}
}

#endif
