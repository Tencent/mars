/*
 * sdt_logic.h
 *
 *  Created on: 2016年3月17日
 *      Author: caoshaokun
 */

#ifndef SDT_INTERFACE_SDT_LOGIC_H_
#define SDT_INTERFACE_SDT_LOGIC_H_

#include "mars/sdt/sdt.h"

namespace mars {
namespace sdt {

	class Callback
	{
	public:
		virtual ~Callback() {};
	};

	void SetCallBack(Callback* const callback);

	void SetHttpNetcheckCGI(std::string cgi);

	//active netcheck interface
	void StartActiveCheck(CheckIPPorts& _longlink_check_item, CheckIPPorts& _shortlink_check_item, int _mode, int _timeout);
	void CancelActiveCheck();

}}

#endif /* SDT_INTERFACE_SDT_LOGIC_H_ */
