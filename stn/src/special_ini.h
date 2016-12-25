//
//  special_ini.h
//  mmcomm
//
//  Created by 刘粲 on 13-12-9.
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
        
		if(INI::Create(des)) {
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
