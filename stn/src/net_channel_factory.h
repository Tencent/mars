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

class ActiveLogic;

namespace mars {
namespace stn {

class LongLink;
class NetSource;
class ShortLinkInterface;

namespace ShortLinkChannelFactory {

ShortLinkInterface* Create(MessageQueue::MessageQueue_t _messagequeueid, NetSource& _netsource, const std::vector<std::string>& _host_list,
					const std::string& _url, const int _taskid, bool _use_proxy);

void Destory(ShortLinkInterface* _short_link_channel);

}

namespace LongLinkChannelFactory {

LongLink* Create(NetSource& _netsource,	MessageQueue::MessageQueue_t _messagequeueid);

void Destory(LongLink* _long_link_channel);

}

}
}



#endif /* SRC_NET_CHANNEL_FACTORY_H_ */
