/*
 * shortlink_packer.h
 *
 *  Created on: 2016年3月15日
 *      Author: yanguoyue
 */

#ifndef SRC_SHORTLINK_PACKER_H_
#define SRC_SHORTLINK_PACKER_H_

#ifdef __APPLE__
#include "mars/comm/http.h"
#else
#include "comm/http.h"
#endif

using namespace http;

class AutoBuffer;

void shortlink_pack(const std::string& _url, const std::map<std::string, std::string>& _headers, const AutoBuffer& _body, AutoBuffer& _out_buff);

#endif /* SRC_SHORTLINK_PACKER_H_ */
