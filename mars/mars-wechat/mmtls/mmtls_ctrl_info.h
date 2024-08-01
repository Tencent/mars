/*
 * MmtlsCtrlInfoLogic.h
 *
 *  Created on: 2016年3月25日
 *      Author: elviswu
 */

#ifndef SRC_MMTLSCTRLINFOLOGIC_H_
#define SRC_MMTLSCTRLINFOLOGIC_H_

#include "mars/boot/context.h"
#include "mars/comm/ini.h"

namespace mars {
namespace comm {
class Mutex;
}

namespace stn {

class MMTLSCtrlInfo {
 public:
    MMTLSCtrlInfo(mars::boot::Context* _context, bool _use_mmtls, const std::string& _filepath);
    virtual ~MMTLSCtrlInfo();

 public:
    void Save(bool _use_mmtls);
    bool IsMMTLSEnabled();
    void ClearAllMMtlsPsk();

 private:
    mars::boot::Context* context_;
    bool use_mmtls;
    INI ini;
};

}  // namespace stn
}  // namespace mars

#endif /* SRC_MMTLSCTRLINFOLOGIC_H_ */
