// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  special_ini.h
//  mmcomm
//
//  Created by liucan on 13-12-9.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#ifndef MMCOMM_SRC_SPECIAL_INI_H_
#define MMCOMM_SRC_SPECIAL_INI_H_

#include "mars/comm/ini.h"
#include "mars/comm/md5.h"
#include "mars/comm/xlogger/xlogger.h"

/**
 *	In order to allow the value of section is anything
 */
class SpecialINI : public INI {
public:
	SpecialINI(const std::string& fileName, bool parse = true) : INI(fileName, parse){
        
	}
    
	bool Create(const std::string& section) {

		unsigned char sig[16] = {0};
		MD5_buffer(section.c_str(), (unsigned int)section.length(), sig);
        
		char des[33] = {0};
		MD5_sig_to_string((const char*)sig, des);
        
		if (INI::Create(des)) {
			Set<std::string>("name", section);
			return true;
		}
        
		return false;
	}
    
	bool Select(const std::string& section) {
        
		unsigned char sig[16] = {0};
		MD5_buffer(section.c_str(), (unsigned int)section.length(), sig);
        
		char des[33] = {0};
		MD5_sig_to_string((const char*)sig, des);
		if (strnlen(des, 33) == 0) {
			xerror2(TSF"section:%0, sig:%1, des:%2", section, sig, des);
		}
		return INI::Select(des);
	}
};

#endif	// MMCOMM_SRC_SPECIAL_INI_H_
