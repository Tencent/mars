/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * dns.h
 *
 *  Created on: 2012-11-23
 *      Author: yanguoyue
 */

#ifndef COMM_COMM_DNS_H_
#define COMM_COMM_DNS_H_

#include <string>
#include <vector>

struct DNSBreaker {
	DNSBreaker(): isbreak(false), dnsstatus(NULL) {}
	bool isbreak;
	int* dnsstatus;
};

class DNS {
  public:
   typedef std::vector<std::string> (*DNSFunc)(const std::string& host);

  public:
    DNS(DNSFunc _dnsfunc=NULL);
    ~DNS();
    
  public:
    bool GetHostByName(const std::string& _host_name, std::vector<std::string>& ips, long millsec = 2 * 1000, DNSBreaker* _breaker = NULL);
    void Cancel(const std::string& _host_name = std::string());
    void Cancel(DNSBreaker& _breaker);
    
  private:
    DNSFunc dnsfunc_;
};


#endif /* COMM_COMM_DNS_H_ */
