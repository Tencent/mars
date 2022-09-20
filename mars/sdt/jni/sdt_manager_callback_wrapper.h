//
// Created by Cpan on 2022/9/20.
//

#ifndef MMNET_SDT_MANAGER_CALLBACK_WRAPPER_H
#define MMNET_SDT_MANAGER_CALLBACK_WRAPPER_H

#include <jni.h>

#include "mars/sdt/sdt.h"

namespace mars {
namespace sdt {
class SdtManagerJniCallback : public Callback {
 public:
    SdtManagerJniCallback(JNIEnv* env, jobject callback);
    virtual ~SdtManagerJniCallback();

 private:
    jobject callback_inst_;
    jclass callbacks_class_;

 public:
    virtual void ReportNetCheckResult(const std::vector<CheckResultProfile>& _check_results) override;
};

}  // namespace sdt
}  // namespace mars

#endif  // MMNET_SDT_MANAGER_CALLBACK_WRAPPER_H
