/*
 * shortlink_packer.h
 *
 *  Created on: 2016-03-15
 *      Author: yanguoyue
 */

#ifndef SRC_SHORTLINK_PACKER_H_
#define SRC_SHORTLINK_PACKER_H_

#ifdef __APPLE__
#include "mars/comm/http.h"
#else
#include "comm/http.h"
#endif


class AutoBuffer;

void shortlink_pack(const std::string& _url, const std::map<std::string, std::string>& _headers, const AutoBuffer& _body, AutoBuffer& _out_buff);

#endif /* SRC_SHORTLINK_PACKER_H_ */
