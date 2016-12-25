/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * httpquery.h
 *
 *  Created on: 2014年6月27日
 *      Author: wutianqiang
 */

#ifndef SDT_SRC_CHECKIMPL_HTTPQUERY_H_
#define SDT_SRC_CHECKIMPL_HTTPQUERY_H_

#include <string>

/**
 *  返回值：0 表示成功 -1表示失败
 *  参数： _url 要发送http请求的目标url
 *  	 recv是目标服务器对该http请求的响应
 *  	 timeout为设置的查询超时时间，单位为ms
 */
int SendHttpQuery(const std::string& _url, int& _status_code, std::string& _errmsg, int _timeout/*ms*/);



#endif /* SDT_SRC_CHECKIMPL_HTTPQUERY_H_ */
