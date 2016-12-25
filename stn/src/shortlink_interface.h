/*
 * shortlink_interface.h
 *
 *  Created on: Jul 21, 2016
 *      Author: wutianqiang
 */

#ifndef SRC_SHORTLINK_INTERFACE_H_
#define SRC_SHORTLINK_INTERFACE_H_

#include "mars/comm/autobuffer.h"
#include "mars/stn/stn.h"
#include "mars/stn/task_profile.h"

namespace mars {
namespace stn {

class ShortLinkInterface {
  public:
	virtual ~ShortLinkInterface(){};

	virtual void            SendRequest(AutoBuffer& _buf_req) = 0;
	virtual ConnectProfile  Profile() const { return ConnectProfile();}

    boost::function<void (int _line, ErrCmdType _errtype, int _errcode, const std::string& _ip, const std::string& _host, uint16_t _port)> func_network_report;
    boost::function<void (ShortLinkInterface* _worker, ErrCmdType _err_type, int _status, AutoBuffer& _body, bool _cancel_retry, ConnectProfile& _conn_profile)> OnResponse;
    boost::function<void (ShortLinkInterface* _worker)> OnSend;
	boost::function<void (ShortLinkInterface* _worker, unsigned int _cached_size, unsigned int _total_size)> OnRecv;
};
    
}
}

#endif /* SRC_SHORTLINK_INTERFACE_H_ */
