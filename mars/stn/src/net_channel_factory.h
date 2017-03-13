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

#include <string>
#include <vector>

#include "mars/comm/messagequeue/message_queue.h"


namespace mars {
namespace stn {

struct Task;
class LongLink;
class NetSource;
class ShortLinkInterface;

namespace ShortLinkChannelFactory {

extern ShortLinkInterface* (*Create)(const mq::MessageQueue_t& _messagequeueid, NetSource& _netsource, const Task& _task, bool _use_proxy);

extern void (*Destory)(ShortLinkInterface* _short_link_channel);

}

namespace LongLinkChannelFactory {

extern LongLink* (*Create)(const mq::MessageQueue_t& _messagequeueid, NetSource& _netsource);

extern void (*Destory)(LongLink* _long_link_channel);

}

}
}



#endif /* SRC_NET_CHANNEL_FACTORY_H_ */
