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
 * net_channel_factory.cc
 *
 *  Created on: Jul 19, 2016
 *      Author: wutianqiang
 */

#include "net_channel_factory.h"

#include "longlink.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/xlogger/xlogger.h"
#include "shortlink.h"

using namespace mars::comm;
namespace mars {
namespace stn {

namespace ShortLinkChannelFactory {

ShortLinkInterface* (*Create)(boot::Context* _context,
                              const mq::MessageQueue_t& _messagequeueid,
                              std::shared_ptr<NetSource> _netsource,
                              const Task& _task,
                              const ShortlinkConfig& _config) =
    [](boot::Context* _context,
       const mq::MessageQueue_t& _messagequeueid,
       std::shared_ptr<NetSource> _netsource,
       const Task& _task,
       const ShortlinkConfig& _config) -> ShortLinkInterface* {
    xdebug2(TSF "use weak func Create");
    return new ShortLink(_context, _messagequeueid, _netsource, _task, _config.use_proxy);
};

void (*Destory)(ShortLinkInterface* _short_link_channel) = [](ShortLinkInterface* _short_link_channel) {
    delete _short_link_channel;
    _short_link_channel = NULL;
};

}  // namespace ShortLinkChannelFactory

namespace LongLinkChannelFactory {

LongLink* (*Create)(boot::Context* _context,
                    const mq::MessageQueue_t& _messagequeueid,
                    std::shared_ptr<NetSource> _netsource,
                    const LonglinkConfig& _config) = [](boot::Context* _context,
                                                        const mq::MessageQueue_t& _messagequeueid,
                                                        std::shared_ptr<NetSource> _netsource,
                                                        const LonglinkConfig& _config) {
    LongLink* longlink = new LongLink(_context, _messagequeueid, _netsource, _config, gDefaultLongLinkEncoder);
    if (_config.dns_func != nullptr) {
        longlink->SetDnsFunc(_config.dns_func);
    }
    return longlink;
};

void (*Destory)(LongLink* _long_link_channel) = [](LongLink* _long_link_channel) {
    delete _long_link_channel;
    _long_link_channel = NULL;
};

}  // namespace LongLinkChannelFactory
}  // namespace stn
}  // namespace mars
