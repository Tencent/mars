/*
 * dns.h
 *
 *  Created on: 2016-12-25
 *      Author: yerungui
 */

#ifndef COMM_COMM_DNS_H_
#define COMM_COMM_DNS_H_

#include <string>
#include <vector>

namespace coroutine {
    
struct DNSBreaker {
	DNSBreaker(): isbreak(false) {}
	bool isbreak;
};

struct dnsinfo;

class DNS {
  public:
   typedef std::vector<std::string> (*DNSFunc)(const std::string& host);

  public:
    DNS(DNSFunc _dnsfunc=NULL);
    ~DNS();
    
  public:
    bool GetHostByName(const std::string& _host_name, std::vector<std::string>& ips, long millsec = 3 * 1000, DNSBreaker* _breaker = NULL);
    void Cancel(const std::string& _host_name = std::string());
    void Cancel(DNSBreaker& _breaker);
    
  private:
    DNSFunc dnsfunc_;
    std::vector<dnsinfo> dnsinfo_vec_;
};
}


#endif /* COMM_COMM_DNS_H_ */
