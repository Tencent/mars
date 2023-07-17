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
 * net_channel_factory.h
 *
 *  Created on: Jul 19, 2016
 *      Author: wutianqiang
 */

#ifndef SRC_NET_CHANNEL_FACTORY_H_
#define SRC_NET_CHANNEL_FACTORY_H_

#include <functional>
#include <string>
#include <vector>

#include "mars/boot/context.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/stn/proto/longlink_packer.h"
#include "mars/stn/stn.h"

namespace mars {
namespace stn {

struct Task;
class LongLink;
class NetSource;
class ShortLinkInterface;

namespace ShortLinkChannelFactory {

extern ShortLinkInterface* (*Create)(boot::Context* _context,
                                     const comm::mq::MessageQueue_t& _messagequeueid,
                                     std::shared_ptr<NetSource> _netsource,
                                     const Task& _task,
                                     const ShortlinkConfig& _config);

extern void (*Destory)(ShortLinkInterface* _short_link_channel);

}  // namespace ShortLinkChannelFactory

namespace LongLinkChannelFactory {

extern LongLink* (*Create)(boot::Context* _context,
                           const comm::mq::MessageQueue_t& _messagequeueid,
                           std::shared_ptr<NetSource> _netsource,
                           const LonglinkConfig& _config);

extern void (*Destory)(LongLink* _long_link_channel);

}  // namespace LongLinkChannelFactory

}  // namespace stn
}  // namespace mars

#endif /* SRC_NET_CHANNEL_FACTORY_H_ */
