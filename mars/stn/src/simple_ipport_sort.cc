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
 *   simple_ipport_sort.cc
 *   network
 *
 *   Created by liucan on 14-6-16.
 *   Copyright (c) 2014年 Tencent. All rights reserved.
*/

#include "simple_ipport_sort.h"

#include <unistd.h>
#include <math.h>
#include <deque>
#include <algorithm>

#include "boost/filesystem.hpp"
#include "boost/bind.hpp"
#include "boost/accumulators/numeric/functional.hpp"

#include "mars/comm/socket/unix_socket.h"

#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/platform_comm.h"

#include "mars/app/app.h"

#define IPPORT_RECORDS_FILENAME "/ipportrecords2.xml"

static const time_t kRecordTimeout = 60 * 60 * 24;
static const char* const kFolderName = "host";
// const static char* const RECORDS = "records";
static const char* const kRecord = "record";
static const char* const kItem = "item";
static const char* const kTime = "time";
static const char* const kNetInfo = "netinfo";
static const char* const kIP = "ip";
static const char* const kPort = "port";
//static const char* const kSuccess = "succ";
//static const char* const kTotal = "total";
static const char* const kHistoryResult = "historyresult";

static const unsigned int kBanTime = 6 * 60 * 1000;  // 6 min
static const unsigned int kMaxBanTime = 30 * 60 * 1000; // 30 min
static const unsigned int kServerBanTime = 30 * 60 * 1000; // 30 min
static const int kBanFailCount = 3;
static const int kSuccessUpdateInterval = 10*1000;
static const int kFailUpdateInterval = 10*1000;

#define SET_BIT(SET, RECORDS)  RECORDS = (((RECORDS)<<1) | (bool(SET)))

static inline
uint32_t CAL_BIT_COUNT(uint64_t RECORDS) {
    uint32_t COUNT = 0;
    while(RECORDS)
    {
        RECORDS = RECORDS & (RECORDS-1);
        COUNT++;
    }
    return COUNT;
}


static inline
uint32_t CAL_LAST_CONTINUOUS_BIT_COUNT(uint64_t RECORDS) {
    uint32_t COUNT = 0;
    uint64_t TMP = RECORDS & 0x1;
    while(TMP)
    {
        COUNT++;
        RECORDS = RECORDS >> 1;
        TMP = RECORDS & 0x1;
    }
    return COUNT;
}

///////////////////////////////////////////////////////////////////////////////////////////
namespace mars { namespace stn {
    struct BanItem {
        std::string ip;
        uint16_t port;
        uint8_t records;
        tickcount_t last_fail_time;
        tickcount_t last_suc_time;
        BanItem(): port(0), records(0) {}
    };
}}

using namespace mars::stn;

SimpleIPPortSort::SimpleIPPortSort()
: hostpath_(mars::app::GetAppFilePath() + "/" + kFolderName)
, IPv6_ban_flag_(0)
, IPv4_ban_flag_(0) 
, ban_v6_(false) {
        
    if (!boost::filesystem::exists(hostpath_)){
        boost::filesystem::create_directory(hostpath_);
    }
        
    ScopedLock lock(mutex_);
    __LoadXml();
    lock.unlock();
    InitHistory2BannedList(false);
}

SimpleIPPortSort::~SimpleIPPortSort() {
    ScopedLock lock(mutex_);
    __SaveXml();
}

void SimpleIPPortSort::__SaveXml() {
    __RemoveTimeoutXml();
    recordsxml_.SaveFile((hostpath_ + IPPORT_RECORDS_FILENAME).c_str());
}

void SimpleIPPortSort::__LoadXml() {
    tinyxml2::XMLError error = recordsxml_.LoadFile((hostpath_ + IPPORT_RECORDS_FILENAME).c_str());
    if (tinyxml2::XML_SUCCESS != error) return;
    __RemoveTimeoutXml();
}

void SimpleIPPortSort::__RemoveTimeoutXml() {
    std::vector<tinyxml2::XMLElement*> remove_ele_ptr_list;

    for (tinyxml2::XMLElement* record = recordsxml_.FirstChildElement(kRecord);
            NULL != record; record = record->NextSiblingElement(kRecord)) {
        const char* lasttime_chr = record->Attribute(kTime);

        if (lasttime_chr) {
            struct timeval now = {0};
            gettimeofday(&now, NULL);
            time_t lasttime = (time_t)strtoul(lasttime_chr, NULL, 10);

            if (now.tv_sec < lasttime || now.tv_sec - lasttime >= kRecordTimeout) {
                remove_ele_ptr_list.push_back(record);
            }
        } else {
            remove_ele_ptr_list.push_back(record);
        }
    }

    for (std::vector<tinyxml2::XMLElement*>::iterator iter = remove_ele_ptr_list.begin();
            iter != remove_ele_ptr_list.end(); ++iter) {
        recordsxml_.DeleteChild(*iter);
    }
}

void SimpleIPPortSort::InitHistory2BannedList(bool _savexml) {
    ScopedLock lock(mutex_);
    if (_savexml) __SaveXml();
    
    _ban_fail_list_.clear();
    
    std::string curr_netinfo;
    if (kNoNet == getCurrNetLabel(curr_netinfo)) return;

    const tinyxml2::XMLElement* record = NULL;

    for (record = recordsxml_.FirstChildElement(kRecord);
            NULL != record; record = record->NextSiblingElement(kRecord)) {
        const char* netinfoChr = record->Attribute(kNetInfo);

        if (netinfoChr && (0 == strcmp(netinfoChr, curr_netinfo.c_str()))) {
            xwarn2(TSF"netinfoChr:%_, curr_netinfo.c_str():%_", netinfoChr, curr_netinfo.c_str());
            break;
        }
    }

    if (NULL == record) { return; }

    for (const tinyxml2::XMLElement* item = record->FirstChildElement(kItem); NULL != item; item = item->NextSiblingElement(kItem)) {
        const char* ip = item->Attribute(kIP);
        uint32_t    port = item->UnsignedAttribute(kPort);
        uint64_t    historyresult = (uint64_t)item->Int64Attribute(kHistoryResult);
        
        struct BanItem banitem;
        if(ip) banitem.ip = ip;
        banitem.port = port;
        banitem.records = 0;
        //8 in 1
        for (int i = 0; i < 8; ++i) {
            SET_BIT(historyresult & 0xFF, banitem.records);
            historyresult >>= 8;
        }
        _ban_fail_list_.push_back(banitem);
    }
}

void SimpleIPPortSort::RemoveBannedList(const std::string& _ip) {
    ScopedLock lock(mutex_);

    for (std::vector<BanItem>::iterator iter = _ban_fail_list_.begin(); iter != _ban_fail_list_.end();) {
        if (iter->ip == _ip)
            iter = _ban_fail_list_.erase(iter);
        else
            ++iter;
    }
}

void SimpleIPPortSort::Update(const std::string& _ip, uint16_t _port, bool _is_success) {
    std::string curr_net_info;
    if (kNoNet == getCurrNetLabel(curr_net_info)) return;

    ScopedLock lock(mutex_);
    
    if (!__CanUpdate(_ip, _port, _is_success)) return;
    
    __UpdateBanList(_is_success,  _ip,  _port);

    tinyxml2::XMLElement* record = NULL;

    for (record = recordsxml_.FirstChildElement(kRecord);
            NULL != record; record = record->NextSiblingElement(kRecord)) {
        const char* netinfo_chr = record->Attribute(kNetInfo);
        if (netinfo_chr && (0 == strcmp(netinfo_chr, curr_net_info.c_str()))) break;
    }

    if (NULL == record) {
        struct timeval timeval = {0};
        gettimeofday(&timeval, NULL);
        char timebuf[128] = {0};
        snprintf(timebuf, sizeof(timebuf), "%ld", timeval.tv_sec);
        
        record = recordsxml_.NewElement(kRecord);
        record->SetAttribute(kNetInfo, curr_net_info.c_str());
        record->SetAttribute(kTime, timebuf);
        recordsxml_.InsertEndChild(record);
    }

    tinyxml2::XMLElement* item = NULL;

    for (item = record->FirstChildElement(kItem);
            NULL != item; item = item->NextSiblingElement(kItem)) {
        const char* ip = item->Attribute(kIP);
        uint32_t port = item->UnsignedAttribute(kPort);

        if (ip && (0 == strcmp(ip, _ip.c_str())) && port == _port) break;
    }

    if (NULL == item) {
        item = recordsxml_.NewElement(kItem);
        item->SetAttribute(kIP, _ip.c_str());
        item->SetAttribute(kPort, _port);
        record->InsertEndChild(item);
    }

    uint64_t history_result = item->Int64Attribute(kHistoryResult);
    SET_BIT(!_is_success, history_result);
    item->SetAttribute(kHistoryResult, (int64_t)history_result);
}

std::vector<BanItem>::iterator  SimpleIPPortSort::__FindBannedIter(const std::string& _ip, unsigned short _port) const {
    std::vector<BanItem>::iterator iter;

    for (iter = _ban_fail_list_.begin(); iter != _ban_fail_list_.end(); ++iter) {
        if (iter->ip == _ip && iter->port == _port) {
            return iter;
        }
    }

    return iter;
}

bool SimpleIPPortSort::__IsBanned(const std::string& _ip, unsigned short _port) const {
    return __IsBanned(__FindBannedIter(_ip, _port));
}

bool SimpleIPPortSort::__IsBanned(std::vector<BanItem>::iterator _iter) const {
    if (_iter == _ban_fail_list_.end()) return false;

    bool baned =  CAL_BIT_COUNT(_iter->records) >= kBanFailCount;
    if (!baned) return false;
    
    uint32_t last_continuous_cnt = CAL_LAST_CONTINUOUS_BIT_COUNT(_iter->records);
    int64_t ban_time = kBanTime;
    if (last_continuous_cnt > kBanFailCount) {
        ban_time += (last_continuous_cnt - kBanFailCount) * kBanTime;
        if (ban_time > kMaxBanTime) {
            ban_time = kMaxBanTime;
        }
        xinfo2(TSF"%_:%_ ban time:%_", _iter->ip, _iter->port, ban_time);
    }
    
    if (_iter->last_fail_time.gettickspan() < ban_time) {
        return true;
    }

    return false;
}

void SimpleIPPortSort::__UpdateBanList(bool _is_success, const std::string& _ip, unsigned short _port) {
    __UpdateBanFlagAndTime(_ip, _is_success);
    for (std::vector<BanItem>::iterator iter = _ban_fail_list_.begin(); iter != _ban_fail_list_.end(); ++iter) {
        if (iter->ip == _ip && iter->port == _port) {
            SET_BIT(!_is_success, iter->records);
            if (_is_success)
                iter->last_suc_time.gettickcount();
            else
                iter->last_fail_time.gettickcount();
            return;
        }
    }

    BanItem item;
    item.ip = _ip;
    item.port = _port;
    SET_BIT(!_is_success, item.records);
    
    if (_is_success)
        item.last_suc_time.gettickcount();
    else
        item.last_fail_time.gettickcount();
    
    _ban_fail_list_.push_back(item);
}


static int DecToBin(int _dec) {
	int result = 0, temp = _dec, j = 1;
	while(temp) {
		result = result + j * (temp % 2);
		temp /= 2;
		j = j * 10;
	}
	return result;
}

void SimpleIPPortSort::__UpdateBanFlagAndTime(const std::string& _ip, bool _success) {
    if (__IsIPv6(_ip)) {
        if (_success) {
            SET_BIT(0, IPv6_ban_flag_);
        } else {
            SET_BIT(1, IPv6_ban_flag_);
        }
        IPv6_ban_flag_ &= 0x7F;
        if (__BanTimes(IPv6_ban_flag_) >= 3) {
            ban_v6_ = true;
        }
    } else {
        if (_success) {
            SET_BIT(0, IPv4_ban_flag_);
        } else {
            SET_BIT(1, IPv4_ban_flag_);
        }
        IPv4_ban_flag_ &= 0x7F;
        if (__BanTimes(IPv4_ban_flag_) >= 3) {
            ban_v6_ = false;
        }
    }
    xdebug2(TSF"ip is %_, success is %_ , current v6 flag %_ , current v4 flag %_", _ip, _success, DecToBin(IPv6_ban_flag_), DecToBin(IPv4_ban_flag_));
}

int SimpleIPPortSort::__BanTimes(uint8_t _flag) {
    int ban_times = 0;
    uint8_t tmp_v6 = _flag;
    while (tmp_v6 & 1) {
        ban_times ++;
        tmp_v6 = tmp_v6 >> 1;
    }
    xinfo2(TSF"flag is %_, ban time is %_ ", _flag, ban_times);
    return ban_times;
}

bool SimpleIPPortSort::CanUseIPv6() {
    return !ban_v6_;
}

bool SimpleIPPortSort::__IsIPv6(const std::string& _ip) {
    in6_addr addr6 = IN6ADDR_ANY_INIT;
    return socket_inet_pton(AF_INET6, _ip.c_str(), &addr6);
}

bool SimpleIPPortSort::__CanUpdate(const std::string& _ip, uint16_t _port, bool _is_success) const {
    for (std::vector<BanItem>::iterator iter = _ban_fail_list_.begin(); iter != _ban_fail_list_.end(); ++iter) {
        if (iter->ip == _ip && iter->port == _port) {
            if (_is_success) {
                return kSuccessUpdateInterval < iter->last_suc_time.gettickspan();
            } else {
                return kFailUpdateInterval < iter->last_fail_time.gettickspan();
            }
        }
    }
    
    return true;
}

void SimpleIPPortSort::__FilterbyBanned(std::vector<IPPortItem>& _items) const {
    for (std::vector<IPPortItem>::iterator it = _items.begin(); it != _items.end();) {
        if (__IsBanned(it->str_ip, it->port) || __IsServerBan(it->str_ip)) {
            xwarn2(TSF"ip:%0, port:%1, is ban!!", it->str_ip, it->port);
            it = _items.erase(it);
        } else {
            ++it;
        }
    }
}

bool SimpleIPPortSort::__IsServerBan(const std::string& _ip) const {
    std::map<std::string, uint64_t>::iterator iter = _server_bans_.find(_ip);

    if (iter == _server_bans_.end()) return false;
    
    uint64_t now = ::gettickcount();
    xassert2(now >= iter->second, TSF"%_:%_", now, iter->second);
    if (now - iter->second < kServerBanTime) {
        xwarn2(TSF"ip %0 is ban by server, haha!", _ip.c_str());
        return true;
    }

    _server_bans_.erase(iter);
    return false;
}

void SimpleIPPortSort::__SortbyBanned(std::vector<IPPortItem>& _items, bool _use_IPv6) const {
    srand((unsigned int)gettickcount());
    //random_shuffle new and history
    std::random_shuffle(_items.begin(), _items.end());

	int cnt = _items.size();
	for (int i = 1; i < cnt - 1; ++i)
	{
		if(_items[i].str_ip == _items[i - 1].str_ip )
		{
			bool find = false;
			for (int j = i + 1; j < cnt; ++j)
			{
				if (_items[i - 1].str_ip != _items[j].str_ip)
				{
					IPPortItem tmp_item = _items[i];
					_items[i] = _items[j];
					_items[j] = tmp_item;
					find = true;
					break;
				}
			}
			if (!find)break;
		}
	}

    //separate new and history
    std::deque<IPPortItem> items_history(_items.size());
    std::deque<IPPortItem> items_new(_items.size());
    auto find_lambda = [&](const IPPortItem& _v) {
        for (std::vector<BanItem>::const_iterator it_banned = _ban_fail_list_.begin(); it_banned != _ban_fail_list_.end(); ++it_banned) {
            if (it_banned->ip == _v.str_ip && it_banned->port == _v.port) {
                return true;
            }
        }
        return false;
    };
    
    items_history.erase(std::remove_copy_if(_items.begin(), _items.end(), items_history.begin(), !boost::bind<bool>(find_lambda, _1)), items_history.end());
    items_new.erase(std::remove_copy_if(_items.begin(), _items.end(), items_new.begin(), find_lambda), items_new.end());
    xassert2(_items.size() == items_history.size()+items_new.size(), TSF"_item:%_, history:%_, new:%_", _items.size(), items_history.size(), items_new.size());
    
    //sort history
    std::sort(items_history.begin(), items_history.end(),
              [&](const IPPortItem& _l, const IPPortItem& _r){
                  auto find_lr_lambda  = [](const BanItem& _v, const IPPortItem& _find) {
                      return _v.ip == _find.str_ip && _v.port == _find.port;
                  };
                      
                  std::vector<BanItem>::const_iterator  l = std::find_if(_ban_fail_list_.begin(), _ban_fail_list_.end(), boost::bind<bool>(find_lr_lambda, _1, _l));
                  std::vector<BanItem>::const_iterator  r = std::find_if(_ban_fail_list_.begin(), _ban_fail_list_.end(), boost::bind<bool>(find_lr_lambda, _1, _r));
                      
                  xassert2(l != _ban_fail_list_.end());
                  xassert2(r != _ban_fail_list_.end());
                  
                 if(l == _ban_fail_list_.end() || r == _ban_fail_list_.end())
                  return false;
                 
                 if (CAL_BIT_COUNT(l->records) != CAL_BIT_COUNT(r->records))
                     return CAL_BIT_COUNT(l->records) < CAL_BIT_COUNT(r->records);
                      
                 if (l->last_fail_time != r->last_fail_time)
                     return l->last_fail_time < r->last_fail_time;
                  
                 if (l->last_suc_time != r->last_suc_time)
                     return l->last_suc_time > r->last_suc_time;
                  
                  //random by std::random_shuffle(_items.begin(), _items.end());
                  return false;
              });
    
   //merge
    _items.clear();

    xinfo2(TSF"use ipv6 %_ ", _use_IPv6);

    //v6 version
    if (!_use_IPv6) {//not use V6
    
        while ( !items_history.empty() || !items_new.empty()) {
            __PickIpItemRandom(_items, items_history, items_new);
        }
        
        return;
    }

    std::deque<IPPortItem> items_new_V6;
    std::deque<IPPortItem> items_new_V4;
    
    for (auto item : items_new) {
        if (__IsV6Ip(item)) {
            items_new_V6.push_back(item);
        } else {
            items_new_V4.push_back(item);
        }
    }

    std::deque<IPPortItem> items_V6_history;
    std::deque<IPPortItem> items_V4_history;
    for (auto item : items_history) {
        if (__IsV6Ip(item)) {
            items_V6_history.push_back(item);
        } else {
            items_V4_history.push_back(item);
        }
    }
    items_history.clear();

    std::deque<IPPortItem>::iterator iterV6 = items_V6_history.begin();
    std::deque<IPPortItem>::iterator iterV4 = items_V4_history.begin();

    while(iterV6 != items_V6_history.end()) {
        items_history.push_back(*iterV6);
        if (iterV4 != items_V4_history.end()) {
            items_history.push_back(*iterV4);
            iterV4 ++;
        }
        iterV6 ++;
    }
    items_history.insert(items_history.end(), iterV4, items_V4_history.end());

    bool pick_V6 = true;    
    while (!items_history.empty() || !items_new_V6.empty() || !items_new_V4.empty()) {
        if (pick_V6) {
            if (!items_history.empty() && __IsV6Ip(items_history.front())) {
                __PickIpItemRandom(_items, items_history, items_new_V6);
            } else { // items_history empty
                if (!items_new_V6.empty()) {
                    _items.push_back(items_new_V6.front());
                    items_new_V6.pop_front();
                }
            }
        } else { //pick v4
            if (!items_history.empty() && !__IsV6Ip(items_history.front())) {
                __PickIpItemRandom(_items, items_history, items_new_V4);
            } else { // items_history empty
                if (!items_new_V4.empty()) {
                    _items.push_back(items_new_V4.front());
                    items_new_V4.pop_front();
                }
            }
        }
        pick_V6 = !pick_V6;
    }

}

bool SimpleIPPortSort::__IsV6Ip(const IPPortItem& item) const {
    return item.str_ip.find(".") == std::string::npos;
}

void SimpleIPPortSort::__PickIpItemRandom(std::vector<IPPortItem>& _items, std::deque<IPPortItem>& _items_history, std::deque<IPPortItem>& _items_new) const {
    int ran = rand() % (_items_history.size() + _items_new.size());
    if (0 <= ran && ran < (int)_items_history.size()) {
        _items.push_back(_items_history.front());
        _items_history.pop_front();
    } else if ((int)_items_history.size() <= ran && ran < (int)(_items_history.size() + _items_new.size())) {
        _items.push_back(_items_new.front());
        _items_new.pop_front();
    } else {
        xassert2(false, TSF"ran:%_, history:%_, new:%_", ran, _items_history.size(), _items_new.size());
    }
}



void SimpleIPPortSort::SortandFilter(std::vector<IPPortItem>& _items, int _needcount, bool _use_IPv6) const {
    xinfo2(TSF"needcount %_, use ipv6 %_ ", _needcount, _use_IPv6);
    ScopedLock lock(mutex_);
    __FilterbyBanned(_items);
    for (size_t i=0; i<_items.size(); i++) {
		xinfo2(TSF"after FilterbyBanned list ip: %_ ", _items[i].str_ip);
	}
    __SortbyBanned(_items, _use_IPv6);

    for (size_t i=0; i<_items.size(); i++) {
		xinfo2(TSF"after SortbyBanned list ip: %_ ", _items[i].str_ip);
	}
    
    if (_needcount < (int)_items.size()) _items.resize(_needcount);
}

void SimpleIPPortSort::AddServerBan(const std::string& _ip) {
    if (_ip.empty()) return;

    ScopedLock lock(mutex_);
    _server_bans_[_ip] = ::gettickcount();
}

