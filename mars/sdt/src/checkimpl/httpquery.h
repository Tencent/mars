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
