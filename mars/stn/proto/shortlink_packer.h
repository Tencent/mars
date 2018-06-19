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
 * shortlink_packer.h
 *
 *  Created on: 2016-03-15
 *      Author: yanguoyue
 */

#ifndef SRC_SHORTLINK_PACKER_H_
#define SRC_SHORTLINK_PACKER_H_

#include <string>
#include <map>

class AutoBuffer;

namespace mars { namespace stn {

class shortlink_tracker {
public:
    static shortlink_tracker* (*Create)();
    
public:
    virtual ~shortlink_tracker(){};
};
    
extern void (*shortlink_pack)(const std::string& _url, const std::map<std::string, std::string>& _headers, const AutoBuffer& _body, const AutoBuffer& _extension, AutoBuffer& _out_buff, shortlink_tracker* _tracker);

}}

#endif /* SRC_SHORTLINK_PACKER_H_ */
