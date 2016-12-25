/*
 * net_channel_factory.cc
 *
 *  Created on: Jul 19, 2016
 *      Author: wutianqiang
 */

#include "net_channel_factory.h"

#include "mars/comm/compiler_util.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/xlogger/xlogger.h"

#include "longlink.h"
#include "shortlink.h"

namespace mars {
namespace stn {

namespace ShortLinkChannelFactory {

WEAK_FUNC ShortLinkInterface* Create(MessageQueue::MessageQueue_t _messagequeueid, NetSource& _netsource, const std::vector<std::string>& _host_list,
					const std::string& _url, const int _taskid, bool _use_proxy) {
	xdebug2(TSF"use weak func Create");
	return new ShortLink(_messagequeueid, _netsource, _host_list, _url, _taskid, _use_proxy);
}

WEAK_FUNC void Destory(ShortLinkInterface* _short_link_channel) {
	delete _short_link_channel;
	_short_link_channel = NULL;
}

}

namespace LongLinkChannelFactory {

WEAK_FUNC LongLink* Create(NetSource& _netsource, MessageQueue::MessageQueue_t _messagequeueid) {
	return new LongLink(_netsource, _messagequeueid);
}

WEAK_FUNC void Destory(LongLink* _long_link_channel) {
	delete _long_link_channel;
	_long_link_channel = NULL;
}

}

}
}
