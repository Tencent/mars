/*
 *   simple_ipport_sort.h
 *   network
 *
 *   Created by 刘粲 on 14-6-16.
 *   Copyright (c) 2014 Tencent. All rights reserved.
*/

#ifndef STN_SRC_SIMPLE_IPPORT_SORT_H_
#define STN_SRC_SIMPLE_IPPORT_SORT_H_

#include <string>
#include <vector>
#include <map>

#include "mars/comm/thread/lock.h"
#include "mars/comm/tinyxml2.h"
#include "mars/comm/tickcount.h"
#include "mars/stn/stn.h"

namespace mars {
namespace stn {

struct BanItem;
    
class SimpleIPPortSort {
  public:
    SimpleIPPortSort();
    ~SimpleIPPortSort();

    void InitHistory2BannedList(bool _savexml);
    void RemoveBannedList(const std::string& _ip);
    void Update(const std::string& _ip, uint16_t _port, bool _is_success);

    void SortandFilter(std::vector<IPPortItem>& _items, int _needcount) const;

    void AddServerBan(const std::string& _ip);
    
  private:
    void __LoadXml();
    void __SaveXml();
    void __RemoveTimeoutXml();

    std::vector<BanItem>::iterator __FindBannedIter(const std::string& _ip, uint16_t _port) const;
    bool __IsBanned(std::vector<BanItem>::iterator _iter) const;
    bool __IsBanned(const std::string& _ip, uint16_t _port) const;
    void __UpdateBanList(bool _isSuccess, const std::string& _ip, uint16_t _port);
    bool __CanUpdate(const std::string& _ip, uint16_t _port, bool _is_success) const;

    void __FilterbyBanned(std::vector<IPPortItem>& _items) const;
    void __SortbyBanned(std::vector<IPPortItem>& _items) const;
    bool __IsServerBan(const std::string& _ip) const;
    
  private:
    SimpleIPPortSort(const SimpleIPPortSort&);
    SimpleIPPortSort& operator=(const SimpleIPPortSort&);

  private:
    std::string hostpath_;
    tinyxml2::XMLDocument recordsxml_;

    mutable Mutex mutex_;
    mutable std::vector<BanItem> _ban_fail_list_;
    mutable std::map<std::string, uint64_t> _server_bans_;
};

}}
#endif // STN_SRC_SIMPLE_IPPORT_SORT_H_
